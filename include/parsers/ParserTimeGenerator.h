/**
 * ParserTimeGenerator.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_TIME_GENERATOR_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_TIME_GENERATOR_H__

#include <stddef.h>

namespace black_library {

namespace core {

namespace parsers {

class ParserTimeGenerator
{
public:
    virtual size_t GenerateWaitTime(const size_t length) = 0;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
