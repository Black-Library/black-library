/**
 * LinkedListParser.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_LINKED_LIST_PARSER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_LINKED_LIST_PARSER_H__

#include "Parser.h"

namespace black_library {

namespace core {

namespace parsers {

class LinkedListParser : public Parser
{
public:
    explicit LinkedListParser(parser_t parser_type, const njson &config);

protected:
    int CalculateIndexBounds(const ParserJob &parser_job);
    void ExpendedAttempts();
    virtual int PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job);
    bool ReachedEnd();
    virtual void SaveLastUrl(ParserResult &parser_result);
    virtual void SaveUpdateDate(ParserResult &parser_result);

    virtual std::string GetFirstUrl(xmlNodePtr root_node, const std::string &data_url) = 0;

    std::string next_url_;
    time_t last_update_date_;
    size_t target_start_index_;
    size_t target_end_index_;
    bool reached_end_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
