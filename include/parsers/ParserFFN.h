/**
 * ParserFFN.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_FFN_PARSER_FFN_H__
#define __BLACK_LIBRARY_CORE_PARSERS_FFN_PARSER_FFN_H__

#include "Parser.h"

namespace black_library {

namespace core {

namespace parsers {

namespace FFN {

class ParserFFN : public Parser
{
public:
    ParserFFN();

protected:
    std::string ParseTitle();
    std::string ParseAuthor();

private:
};

} // namespace FFN

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
