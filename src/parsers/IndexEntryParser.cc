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
    index_entries_()
{
    parser_behavior_ = parser_behavior_t::INDEX_ENTRY;
}

int IndexEntryParser::CalculateIndexBounds(const ParserJob &parser_job)
{
    if (parser_job.start_number > index_entries_.size())
    {
        index_ = 0;
    }
    else
    {
        index_ = parser_job.start_number - 1;
    }

    if (parser_job.start_number <= parser_job.end_number && index_entries_.size() >= parser_job.end_number)
    {
        end_index_ = parser_job.end_number - 1;
    }
    else
    {
        end_index_ = index_entries_.size() - 1;
    }

    if (index_ > index_entries_.size() || index_entries_.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Requested start index {} greater than detected entries size: {}", index_, index_entries_.size());
        return -1;
    }

    return 0;
}

void IndexEntryParser::ExpendedAttempts()
{
    ++index_;
}

int IndexEntryParser::PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job)
{
    BlackLibraryCommon::LogDebug(parser_name_, "Find index entry nodes");

    FindIndexEntries(root_node);

    BlackLibraryCommon::LogDebug(parser_name_, "Found {} nodes", index_entries_.size());

    if (index_entries_.size() <= 0 && index_entries_.size() < parser_job.start_number)
    {
        BlackLibraryCommon::LogError(parser_name_, "Index entries size error");
        return -1;
    }

    return 0;
}

bool IndexEntryParser::ReachedEnd()
{
    return index_ > end_index_;
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
