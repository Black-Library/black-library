/**
 * ParserAO3.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_AO3_PARSER_AO3_H__
#define __BLACK_LIBRARY_CORE_PARSERS_AO3_PARSER_AO3_H__

#include "IndexEntryParser.h"

namespace black_library {

namespace core {

namespace parsers {

namespace AO3 {

class ParserAO3 : public IndexEntryParser
{
public:
    explicit ParserAO3(const njson &config);
    ~ParserAO3();

protected:
    ParserIndexEntry ExtractIndexEntry(xmlNodePtr root_node);
    void FindIndexEntries(xmlNodePtr root_node);
    void FindMetaData(xmlNodePtr root_node);
    virtual ParseSectionInfo ParseSection();
    std::string PreprocessTargetUrl(const ParserJob &parser_job);

    ParserTimeResult GetPublishedTime(xmlNodePtr root_node);

private:
    
};

} // namespace AO3

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
