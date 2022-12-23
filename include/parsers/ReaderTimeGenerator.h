/**
 * ReaderTimeGenerator.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_READER_TIME_GENERATOR_H__
#define __BLACK_LIBRARY_CORE_PARSERS_READER_TIME_GENERATOR_H__

#include <time.h>

#include <random>

#include "ParserTimeGenerator.h"

namespace black_library {

namespace core {

namespace parsers {

class ReaderTimeGenerator : public ParserTimeGenerator
{
public:
    ReaderTimeGenerator();

    size_t GenerateWaitTime(const size_t length) override;

private:
    std::mt19937_64 generator_;
    std::uniform_int_distribution<int> distribution_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
