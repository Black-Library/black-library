/**
 * TimeOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_TIME_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_TIME_OPERATIONS_H__

#include <ctime>
#include <string>

namespace black_library {

namespace core {

namespace common {

std::string GetGUITimeString(const time_t &time);
std::string GetISOTimeString(const time_t &time);
std::time_t GetUnixTime();
std::time_t ParseTimet(const std::string &input, const std::string &format);

} // namespace common
} // namespace core
} // namespace black_library

#endif
