/**
 * ConfigOperations.cc
 */

#include <iostream>

#include <ConfigOperations.h>

namespace black_library {

namespace core {

namespace common {

njson LoadConfig(const njson &config)
{
    if (!config.contains("config"))
    {
        std::cout << "config load failed: " << config.size() << std::endl;
        exit(1);
    }
    return config["config"];
}

} // namespace common
} // namespace core
} // namespace black_library
