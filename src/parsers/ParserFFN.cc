/**
 * ParserFFN.cc
 */

#include <ParserFFN.h>

namespace black_library {

namespace core {

namespace parsers {

namespace FFN {

ParserFFN::ParserFFN()
{
    parser_type_ = FFN_PARSER;
    source_url_ = FF::source_url;
}

ParserFFN::~ParserFFN()
{

}

std::string ParserFFN::ParseTitle()
{
    return "";
}

std::string ParserFFN::ParseAuthor()
{
    return "";
}

} // namespace FFN

} // namespace parsers
} // namespace core
} // namespace black_library
