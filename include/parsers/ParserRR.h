/**
 * ParserRR.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_RR_PARSER_RR_H__
#define __BLACK_LIBRARY_CORE_PARSERS_RR_PARSER_RR_H__

#include "IndexEntryParser.h"

namespace black_library {

namespace core {

namespace parsers {

namespace RR {

class ParserRR : public IndexEntryParser
{
public:
    explicit ParserRR(const njson &config);

protected:
    std::string GetRRIndexEntryTitle(const ParserIndexEntry &index_entry);

    ParserIndexEntry ExtractIndexEntry(xmlNodePtr root_node);
    void FindIndexEntries(xmlNodePtr root_node);
    void FindMetaData(xmlNodePtr root_node);
    ParseSectionInfo ParseSection();
    ParserXmlNodeSeek SeekToIndexEntryContent(xmlNodePtr root_node);

};

} // namespace RR
} // namespace parsers
} // namespace core
} // namespace black_library

#endif
