/**
 * ParserSVF.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_SVF_PARSER_SVF_H__
#define __BLACK_LIBRARY_CORE_PARSERS_SVF_PARSER_SVF_H__

#include "Parser.h"
#include "ParserXF.h"

namespace black_library {

namespace core {

namespace parsers {

namespace SVF {

class ParserSVF : public XF::ParserXF
{
public:
    explicit ParserSVF(const njson &config);

};

} // namespace SVF
} // namespace parsers
} // namespace core
} // namespace black_library

#endif
