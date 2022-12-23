/**
 * ParserWP.h
 *
 * WordPress parser
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_WP_PARSER_WP_H__
#define __BLACK_LIBRARY_CORE_PARSERS_WP_PARSER_WP_H__

#include "LinkedListParser.h"

namespace black_library {

namespace core {

namespace parsers {

namespace WP {

class ParserWP : public LinkedListParser
{
public:
    explicit ParserWP(const njson &config);

protected:
    void FindMetaData(xmlNodePtr root_node);
    ParseSectionInfo ParseSection();
    std::string PreprocessTargetUrl(const ParserJob &parser_job);

    std::string GetFirstUrl(xmlNodePtr root_node, const std::string &data_url) override;

    std::string GetNextUrl(xmlNodePtr root_node);
    std::string GetSectionTitle(xmlNodePtr root_node);
    time_t GetUpdateDate(xmlNodePtr root_node);
};

} // namespace WP
} // namespace parsers
} // namespace core
} // namespace black_library

#endif
