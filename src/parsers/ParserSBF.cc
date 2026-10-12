/**
 * ParserSBF.cc
 */

#include <ParserSBF.h>

namespace black_library {

namespace core {

namespace parsers {

namespace SBF {

namespace BlackLibraryCommon = black_library::core::common;

ParserSBF::ParserSBF(const njson &config) :
    ParserXF(parser_t::SBF_PARSER, config)
{
    source_name_ = BlackLibraryCommon::SBF::source_name;
    source_url_ = BlackLibraryCommon::SBF::source_url;

    xf_version_ = "2.3";
}

} // namespace SBF
} // namespace parsers
} // namespace core
} // namespace black_library
