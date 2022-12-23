/**
 * ReaderTimeGenerator.cc
 */

#include <chrono>

#include <ReaderTimeGenerator.h>

namespace black_library {

namespace core {

namespace parsers {

ReaderTimeGenerator::ReaderTimeGenerator()
{
    std::random_device rd;
    std::mt19937_64::result_type seed = rd() ^ (
            (std::mt19937_64::result_type)
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count() +
            (std::mt19937_64::result_type)
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count() );

    generator_ = std::mt19937_64(seed);
    distribution_ = std::uniform_int_distribution<int>(0, 2);
}

// TODO: make parser take 8ish hour break
size_t ReaderTimeGenerator::GenerateWaitTime(const size_t length)
{
    size_t wait_time = 0;

    size_t loops = length;

    if (length <= 20)
        loops = 40;

    for (size_t i = 0; i < loops; ++i)
    {
        wait_time += 3 + distribution_(generator_);
    }

    return wait_time;
}

} // namespace parsers
} // namespace core
} // namespace black_library
