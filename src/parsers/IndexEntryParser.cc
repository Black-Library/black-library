/**
 * IndexEntryParser.cc
 */

#include <functional>

#include <LogOperations.h>

#include <IndexEntryParser.h>
#include <ShortTimeGenerator.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

IndexEntryParser::IndexEntryParser(parser_t parser_type, const njson &config) : 
    Parser(parser_type, config),
    index_entries_(),
    index_entry_queue_(),
    last_url_(),
    check_date_enabled_(false),
    entry_gap_(false)
{
    parser_behavior_ = parser_behavior_t::INDEX_ENTRY;
}

int IndexEntryParser::CalculateIndexBounds(const ParserJob &parser_job)
{
    index_ = 0;

    target_start_index_ = parser_job.start_number - 1;
    target_end_index_ = parser_job.end_number - 1;

    if (target_end_index_ == 0)
    {
        target_end_index_ = std::numeric_limits<size_t>::max();
    }

    return 0;
}

void IndexEntryParser::ExpendedAttempts()
{
    // if expended attempts, clear queue
    std::queue<ParserIndexEntry> empty;
    index_entry_queue_.swap(empty);
}

void IndexEntryParser::IndicateNextSection()
{
    if (index_entry_queue_.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Index entry parser tried to pop empty queue");
        return;
    }

    // update last update date if next in queue is more recent
    if (last_update_date_ < index_entry_queue_.front().time_published)
        last_update_date_ = index_entry_queue_.front().time_published;
    last_url_ = index_entry_queue_.front().data_url;

    index_entry_queue_.pop();
}

int IndexEntryParser::PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job)
{
    (void) parser_job;
    BlackLibraryCommon::LogDebug(parser_name_, "Find index entry nodes with UUID: {}", uuid_);

    FindIndexEntries(root_node);

    if (db_adapter_)
        md5s_ = db_adapter_->ReadMd5s(uuid_);

    BlackLibraryCommon::LogDebug(parser_name_, "Found {} nodes and {} db md5s for {}", index_entries_.size(), md5s_.size(), uuid_);

    if (index_entries_.size() <= 0)
    {
        BlackLibraryCommon::LogError(parser_name_, "Index entries size error with UUID: {}", uuid_);
        return -1;
    }

    BlackLibraryCommon::LogDebug(parser_name_, "Match index entries to md5s");

    if (md5s_.size() <= 0)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "No md5s for {}, md5 list empty", uuid_);
    }

    // get largest index_num for last update date and url
    std::priority_queue<BlackLibraryCommon::Md5Sum, std::vector<BlackLibraryCommon::Md5Sum>, BlackLibraryCommon::Md5SumGreaterThanBySeqNum> md5_seq_num_queue;
    size_t max_index = 0;
    for (const auto & md5 : md5s_)
    {
        if (md5.index_num >= max_index)
        {
            max_index = md5.index_num;
            last_update_date_ = md5.date;
        }
        if (md5.seq_num == BlackLibraryCommon::MaxSeqNum)
        {
            BlackLibraryCommon::LogWarn(parser_name_, "md5 has MaxSeqNum as seq_num!: {}", md5);
            continue;
        }
        // BlackLibraryCommon::LogDebug(parser_name_, "pushing {} into queue", md5);
        md5_seq_num_queue.push(md5);
    }

    BlackLibraryCommon::LogDebug(parser_name_, "md5_seq_num_queue size: {}", md5_seq_num_queue.size());

    std::vector<ParserIndexEntry> truncated_index_entries;
    std::priority_queue<ParserIndexEntry, std::vector<ParserIndexEntry>, ParserIndexEntryGreaterThanBySeqNum> index_entry_seq_num_queue;
    for (const auto & index_entry : index_entries_)
    {
        index_entry_seq_num_queue.push(index_entry);
    }

    size_t expected_index = 0;
    bool warn_about_order = false;
    while (!md5_seq_num_queue.empty() || !index_entry_seq_num_queue.empty())
    {
        size_t md5_seq_num = BlackLibraryCommon::MaxSeqNum;
        size_t index_entry_seq_num = BlackLibraryCommon::MaxSeqNum;
        bool replace_md5_seq_num = false;
        if (!md5_seq_num_queue.empty())
        {
            md5_seq_num = md5_seq_num_queue.top().seq_num;
            if (md5_seq_num == BlackLibraryCommon::MaxSeqNum)
                replace_md5_seq_num = true;
        }
        if (!index_entry_seq_num_queue.empty())
        {
            std::string index_entry_data_url = index_entry_seq_num_queue.top().data_url;
            index_entry_seq_num = BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(index_entry_data_url);
        }
        if (md5_seq_num == index_entry_seq_num && !replace_md5_seq_num)
        {
            BlackLibraryCommon::Md5Sum md5_sum = md5_seq_num_queue.top();
            if (md5_sum.index_num != expected_index)
            {
                warn_about_order = true;
                BlackLibraryCommon::LogDebug(parser_name_, "eq unexpected md5 index number UUID: {} index_num: {}, expected: {} {}", uuid_, md5_sum.index_num, expected_index, md5_sum);
                // change md5 index_num to match
                md5_sum.index_num = expected_index;
                db_adapter_->UpdateMd5BySeqNum(md5_sum);
            }
            if (md5_seq_num_queue.empty() || index_entry_seq_num_queue.empty())
                BlackLibraryCommon::LogError(parser_name_, "attempting to pop empty md5_seq_num_queue and index_entry_seq_num_queue!");
            md5_seq_num_queue.pop();
            index_entry_seq_num_queue.pop();
        }
        else if (md5_seq_num < index_entry_seq_num && !replace_md5_seq_num)
        {
            BlackLibraryCommon::Md5Sum md5_sum = md5_seq_num_queue.top();
            if (md5_sum.index_num != expected_index)
            {
                warn_about_order = true;
                BlackLibraryCommon::LogDebug(parser_name_, "lt unexpected md5 index number UUID: {} index_num: {}, expected: {} {}", uuid_, md5_sum.index_num, expected_index, md5_sum);
            }
            if (md5_seq_num_queue.empty())
                BlackLibraryCommon::LogError(parser_name_, "attempting to pop empty md5_seq_num_queue!");
            md5_seq_num_queue.pop();        
        }
        else
        {
            ParserIndexEntry index_entry = index_entry_seq_num_queue.top();
            index_entry.index_num = expected_index;
            truncated_index_entries.emplace_back(index_entry);
            if (index_entry_seq_num_queue.empty())
            {
                for (const auto & truncated : truncated_index_entries)
                {
                    BlackLibraryCommon::LogError(parser_name_, "truncated: {}", truncated.data_url);
                }
                BlackLibraryCommon::LogError(parser_name_, "attempting to pop empty index_entry_seq_num_queue! {} and {} - flag: {} {} vs {}", index_entry_seq_num_queue.size(), md5_seq_num_queue.size(), replace_md5_seq_num, md5_seq_num, index_entry_seq_num);
            }
            index_entry_seq_num_queue.pop();
        }
        ++expected_index;
    }
    

    BlackLibraryCommon::LogDebug(parser_name_, "Truncated UUID: {} truncated size: {}, index entries size: {}, md5_sum size: {}", uuid_, truncated_index_entries.size(), index_entries_.size(), md5s_.size());

    if (warn_about_order)
    {
        BlackLibraryCommon::LogWarn(parser_name_, "UUID: {} has ordering issues", uuid_);
        for (const auto & truncated : truncated_index_entries)
        {
            BlackLibraryCommon::LogDebug(parser_name_, "UUID: {} truncated index_num: {}", uuid_, truncated.index_num);
        }
    }

    index_entries_ = truncated_index_entries;

    for (const auto & index_entry : index_entries_)
    {
        index_entry_queue_.emplace(index_entry);
    }

    BlackLibraryCommon::LogDebug(parser_name_, "index_entry_queue_ size: {}", index_entry_queue_.size());

    return 0;
}

bool IndexEntryParser::ReachedEnd()
{
    return index_entry_queue_.empty();
}

void IndexEntryParser::SaveLastUrl(ParserResult &parser_result)
{
    parser_result.metadata.last_url = last_url_;
}

void IndexEntryParser::SaveUpdateDate(ParserResult &parser_result)
{
    parser_result.metadata.update_date = last_update_date_;

    if (parser_result.metadata.update_date <= 0)
        BlackLibraryCommon::LogError(parser_name_, "Failed to get update date for UUID: {}, last update date: {}", uuid_, last_update_date_);
}

} // namespace parsers
} // namespace core
} // namespace black_library
