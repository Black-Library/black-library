/**
 * VersionOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__

#include <string>
#include <vector>

namespace black_library {

namespace core {

namespace common {

static constexpr const char EmptyMD5Version[] = "NO_MD5_VERSION";

std::string GetMD5Hash(const std::string &input);

} // namespace common
} // namespace core
} // namespace black_library

#endif
