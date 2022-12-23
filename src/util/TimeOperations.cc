/**
 * TimeOperations.cc
 */

#include <iomanip>
#include <sstream>

#include <TimeOperations.h>

namespace black_library {

namespace core {

namespace common {

std::string GetGUITimeString(const time_t &time)
{
    std::stringstream str;
    str << std::put_time(std::gmtime(&time), "%F %H:%M") << std::ends;
    return str.str();
}

std::string GetISOTimeString(const time_t &time)
{
    std::stringstream str;
    str << std::put_time(std::gmtime(&time), "%FT%TZ") << std::ends;
    return str.str();
}

std::time_t GetUnixTime()
{
    return std::time(nullptr);
}

std::time_t ParseTimet(const std::string &input, const std::string &format)
{
    struct tm tm_l {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    strptime(input.c_str(), format.c_str(), &tm_l);
    return mktime(&tm_l);
}

} // namespace common
} // namespace core
} // namespace black_library
