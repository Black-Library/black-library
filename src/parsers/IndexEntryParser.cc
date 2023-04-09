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
    index_entry_queue_()
{
    parser_behavior_ = parser_behavior_t::INDEX_ENTRY;
}

int IndexEntryParser::CalculateIndexBounds(const ParserJob &parser_job)
{
    (void) parser_job;
    index_ = 0;

    // new entry with no md5s in db, add everything to queue
    if (md5s_.size() <= 0)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "No md5s for {}, treating as new entry", uuid_);
    }
    // check index entries for already existing values
    else
    {
        // get max index num
        size_t max_index_num = 0;
        for (auto const & index_entry : index_entries_)
        {
            if (max_index_num <= index_entry.index_num)
                max_index_num = index_entry.index_num;
        }

        // max_index_num increment assuming at least one value 
        ++max_index_num;

        // add things to queue that do not have a data_url
        std::vector<ParserIndexEntry> clean_index_entries;
        size_t matched_entries_count = 0;
        for (auto & index_entry : index_entries_)
        {
            // url and date is in md5 map do not add to queue
            if (md5s_.count(index_entry.data_url))
            {
                // check date
                BlackLibraryCommon::Md5Sum md5_sum = md5s_.find(index_entry.data_url)->second;
                if (md5_sum.date == index_entry.time_published)
                {
                    ++matched_entries_count;
                }
                // add to queue, entry requires update
                else
                {
                    clean_index_entries.emplace_back(index_entry);
                }
            }
            // url and date not in md5 map
            else
            {
                index_entry.index_num = index_entry.index_num + max_index_num;
                // skip if index num is less then parser job start num
                clean_index_entries.emplace_back(index_entry);
            }
        }
        index_entries_ = clean_index_entries;
        BlackLibraryCommon::LogDebug(parser_name_, "New index entries size {}, matched index entries size {}", index_entries_.size(), matched_entries_count);
    }

    for (auto const & index_entry : index_entries_)
    {
        index_entry_queue_.push(index_entry);
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
    index_entry_queue_.pop();
}

int IndexEntryParser::PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job)
{
    (void) parser_job;
    BlackLibraryCommon::LogDebug(parser_name_, "Find index entry nodes");

    FindIndexEntries(root_node);

    BlackLibraryCommon::LogDebug(parser_name_, "Found {} nodes", index_entries_.size());

    if (md5s_read_callback_)
        md5s_ = md5s_read_callback_(uuid_);

    if (index_entries_.size() <= 0)
    {
        BlackLibraryCommon::LogError(parser_name_, "Index entries size error");
        return -1;
    }

    return 0;
}

bool IndexEntryParser::ReachedEnd()
{
    return index_entry_queue_.empty();
}

void IndexEntryParser::SaveLastUrl(ParserResult &parser_result)
{
    parser_result.metadata.last_url = index_entries_[index_entries_.size() - 1].data_url;
}

void IndexEntryParser::SaveUpdateDate(ParserResult &parser_result)
{
    for (const auto & index_entry : index_entries_)
    {
        if (index_entry.time_published > parser_result.metadata.update_date)
            parser_result.metadata.update_date = index_entry.time_published;
    }

    if (parser_result.metadata.update_date <= 0)
        BlackLibraryCommon::LogError(parser_name_, "Failed to get update date for UUID: {}", uuid_);
}

} // namespace parsers
} // namespace core
} // namespace black_library
