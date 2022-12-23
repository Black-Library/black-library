/**
 * ParserSBF.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_SBF_PARSER_SBF_H__
#define __BLACK_LIBRARY_CORE_PARSERS_SBF_PARSER_SBF_H__

#include "Parser.h"
#include "ParserXF.h"

namespace black_library {

namespace core {

namespace parsers {

namespace SBF {

class ParserSBF : public XF::ParserXF
{
public:
    explicit ParserSBF(const njson &config);

};

} // namespace SBF
} // namespace parsers
} // namespace core
} // namespace black_library

#endif
