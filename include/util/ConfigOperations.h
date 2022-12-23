/**
 * ConfigOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_CONFIG_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_CONFIG_OPERATIONS_H__

#include <nlohmann/json.hpp>

using njson = nlohmann::json;

namespace black_library {

namespace core {

namespace common {

njson LoadConfig(const njson &config);

} // namespace common
} // namespace core
} // namespace black_library

#endif
