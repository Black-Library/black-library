/**
 * TestIndexEntryParser.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_TEST_INDEX_ENTRY_PARSER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_TEST_INDEX_ENTRY_PARSER_H__

#include "IndexEntryParser.h"

namespace black_library {

namespace core {

namespace parsers {

class TestIndexEntryParser : public IndexEntryParser
{
public:
    explicit TestIndexEntryParser(const njson &config);

    size_t GetIndex();
    size_t GetTargetStartIndex();
    size_t GetTargetEndIndex();

    int TestCalculateIndexBounds(const ParserJob &parser_job);
    void TestExpendedAttempts();
    void TestIndicateNextsection();
    void TestPreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job);
    void TestReachedEnd();
    void TestSaveLastUrl(ParserResult &parser_result);
    void TestSaveUpdate(ParserResult &parser_result);

    std::string TestGetIndexEntryTitle(const ParserIndexEntry &index_entry);

    ParserIndexEntry TestExtractIndexEntry(xmlNodePtr root_node);
    void TestFindIndexEntries(xmlNodePtr root_node);
    void TestFindMetaData(xmlNodePtr root_node);
    ParseSectionInfo TestParseSection();
    ParserXmlNodeSeek TestSeekToIndexEntryContent(xmlNodePtr root_node);

protected:
    std::string GetIndexEntryTitle(const ParserIndexEntry &index_entry);

    ParserIndexEntry ExtractIndexEntry(xmlNodePtr root_node);
    void FindIndexEntries(xmlNodePtr root_node);
    void FindMetaData(xmlNodePtr root_node);
    ParseSectionInfo ParseSection();
    ParserXmlNodeSeek SeekToIndexEntryContent(xmlNodePtr root_node);

};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
