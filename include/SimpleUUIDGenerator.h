/**
 * SimpleUUIDGenerator.h
 */

#ifndef __BLACK_LIBRARY_SIMPLEUUIDGENERATOR_H__
#define __BLACK_LIBRARY_SIMPLEUUIDGENERATOR_H__

#include <random>

namespace black_library {

class SimpleUUIDGenerator
{
public:
    explicit SimpleUUIDGenerator();

    SimpleUUIDGenerator &operator = (SimpleUUIDGenerator &&) = default;

    std::string GenerateUUID();

private:
    std::random_device rd_;
    std::mt19937_64 gen_;
    std::uniform_int_distribution<> dist0_;
    std::uniform_int_distribution<> dist1_;
};

} // namespace black_library

#endif
