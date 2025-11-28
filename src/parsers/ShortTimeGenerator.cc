/**
 * ShortTimeGenerator.cc
 */

#include <ShortTimeGenerator.h>

namespace black_library {

namespace core {

namespace parsers {

ShortTimeGenerator::ShortTimeGenerator()
{
}

size_t ShortTimeGenerator::GenerateWaitTime(const size_t length)
{
    (void) length;

    return 6;
}

} // namespace parsers
} // namespace core
} // namespace black_library
