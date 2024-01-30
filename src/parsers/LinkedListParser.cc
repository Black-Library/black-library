/**
 * LinkedListParser.cc
 */

#include <iostream>
#include <limits>

#include <LogOperations.h>

#include <LinkedListParser.h>
#include <ShortTimeGenerator.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

LinkedListParser::LinkedListParser(parser_t parser_type, const njson &config) : 
    Parser(parser_type, config),
    next_url_(),
    reached_end_(false)
{
    parser_behavior_ = parser_behavior_t::LINKED_LIST;
}

int LinkedListParser::CalculateIndexBounds(const ParserJob &parser_job)
{
    target_start_index_ = parser_job.start_number - 1;
    target_end_index_ = parser_job.end_number - 1;

    if (seq_num_missing_)
        target_start_index_ = 0;

    if (target_end_index_ == 0 || seq_num_missing_)
    {
        target_end_index_ = std::numeric_limits<size_t>::max();
    }

    return 0;
}

void LinkedListParser::ExpendedAttempts()
{
    reached_end_ = true;
}

void LinkedListParser::IndicateNextSection()
{
    ++index_;
}

int LinkedListParser::PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job)
{
    if (parser_job.url == parser_job.last_url || parser_job.last_url.empty() || seq_num_missing_)
        next_url_ = GetFirstUrl(root_node, parser_job.url);
    else
    {
        next_url_ = parser_job.last_url;
        index_ = parser_job.start_number - 1;
    }

    if (next_url_.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "PreParseLoop empty first url");
        return -1;
    }

    BlackLibraryCommon::LogDebug(parser_name_, "next_url_: {}", next_url_);

    return 0;
}

bool LinkedListParser::ReachedEnd()
{
    return reached_end_;
}

void LinkedListParser::SaveLastUrl(ParserResult &parser_result)
{
    if (next_url_.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "save last url empty: {}", uuid_);
        parser_result.metadata.last_url = parser_result.metadata.url;
        return;
    }

    parser_result.metadata.last_url = next_url_;
}

void LinkedListParser::SaveUpdateDate(ParserResult &parser_result)
{
    parser_result.metadata.update_date = last_update_date_;
}

} // namespace parsers
} // namespace core
} // namespace black_library
