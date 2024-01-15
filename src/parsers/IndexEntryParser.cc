/**
 * IndexEntryParser.cc
 */

#include <functional>
#include <iostream>

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
    gap_width_(0),
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
    size_t expected_index = 0;
    size_t max_index = 0;
    for (const auto & md5 : md5s_)
    {
        if (md5.second.index_num >= max_index)
        {
            max_index = md5.second.index_num;
            last_update_date_ = md5.second.date;
        }
        ++expected_index;
    }

    // size_t expected_index = 0;
    // std::vector<ParserIndexEntry> list_of_index_entries;
    // for (const auto & md5 : md5s_)
    // {
    //     // if md5 data url in index_entries, okay to skip, otherwise add to gap width
    //     if !(DoesUrlExistInIndexEntryVector(index_entries_, md5))
    //         ++gap_width_;
    //     // if index_entry url not in md5s check if expected index is less than size of md5s
    //         // if less -> new version
    //         // if more -> new entry should use expected_index
    //     ++expected_index;

    BlackLibraryCommon::LogDebug(parser_name_, "Info from md5s_ UUID: {} md5_index_num_offset: {}", uuid_, gap_width_);

    std::vector<ParserIndexEntry> truncated_index_entries;
    for (const auto & index_entry : index_entries_)
    {
        if (md5s_.count(index_entry.data_url))
        {
            if (md5s_.find(index_entry.data_url)->second.index_num != index_entry.index_num)
            {
                entry_gap_ = true;
                ++gap_width_;
            }
            if (md5s_.find(index_entry.data_url)->second.date == index_entry.time_published)
            {
                continue;
            }
        }

        ParserIndexEntry truncated = index_entry;
        truncated.index_num = index_entry.index_num + gap_width_;

        truncated_index_entries.emplace_back(truncated);
    }

    BlackLibraryCommon::LogWarn(parser_name_, "Truncated UUID: {} truncated size: {}, index entries size: {}", uuid_, truncated_index_entries.size(), index_entries_.size());
    BlackLibraryCommon::LogWarn(parser_name_, "UUID: {} - total entry gap width: {}", uuid_, gap_width_);

    for (const auto & truncated : truncated_index_entries)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "UUID: {} - {}", uuid_, truncated.data_url);
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
