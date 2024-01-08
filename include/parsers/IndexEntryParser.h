/**
 * IndexEntryParser.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_INDEX_ENTRY_PARSER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_INDEX_ENTRY_PARSER_H__

#include <queue>

#include "Parser.h"

namespace black_library {

namespace core {

namespace parsers {

class IndexEntryParser : public Parser
{
public:
    explicit IndexEntryParser(parser_t parser_type, const njson &config);

protected:
    int CalculateIndexBounds(const ParserJob &parser_job);
    void ExpendedAttempts();
    void IndicateNextSection();
    int PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job) override;
    bool ReachedEnd();
    void SaveLastUrl(ParserResult &parser_result);
    void SaveUpdateDate(ParserResult &parser_result);

    virtual ParserIndexEntry ExtractIndexEntry(xmlNodePtr root_node) = 0;
    virtual void FindIndexEntries(xmlNodePtr root_node) = 0;

    std::vector<ParserIndexEntry> index_entries_;
    std::queue<ParserIndexEntry> index_entry_queue_;
    std::string last_url_;
    size_t gap_width_;
    bool check_date_enabled_;
    bool entry_gap_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
