/**
 * ShortTimeGenerator.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_READER_TIME_GENERATOR_H__
#define __BLACK_LIBRARY_CORE_PARSERS_READER_TIME_GENERATOR_H__

#include "ParserTimeGenerator.h"

namespace black_library {

namespace core {

namespace parsers {

class ShortTimeGenerator : public ParserTimeGenerator
{
public:
    ShortTimeGenerator();

    size_t GenerateWaitTime(const size_t length) override;

private:
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
