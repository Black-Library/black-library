/**
 * SimpleUUIDGenerator.cc
 */

#include <sstream>

#include <SimpleUUIDGenerator.h>

namespace black_library {

// namespace BlackLibraryCommon = black_library::core::common;

SimpleUUIDGenerator::SimpleUUIDGenerator() :
    rd_(),
    gen_(),
    dist0_(0, 15),
    dist1_(8, 11)
{
    gen_ = std::mt19937_64(rd_());
}

// https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
std::string SimpleUUIDGenerator::GenerateUUID()
{
    std::stringstream ss;
    size_t i;

    ss << std::hex;
    for (i = 0; i < 8; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    for (i = 0; i < 4; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-4";
    for (i = 0; i < 3; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    ss << dist1_(gen_);
    for (i = 0; i < 3; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    for (i = 0; i < 12; ++i)
    {
        ss << dist0_(gen_);
    }

    return ss.str();
}

} // namespace black_library
