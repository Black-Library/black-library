/**
 * config_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <ConfigOperations.h>

namespace black_library {

namespace core {

namespace common {

njson GenerateEmptyConfig()
{
    njson j;
    return j;
}

njson GenerateFlatTestConfig()
{
    njson j;

    j["config"] = "flat";

    return j;
}

TEST_CASE( "Load config generic tests (pass)", "[single-file]" )
{
    auto empty_config = GenerateEmptyConfig();
    auto flat_config = GenerateFlatTestConfig();
    // TODO: empty config leads to exit, perhaps perfer warning with default config instead
    // REQUIRE( LoadConfig(empty_config) );
    REQUIRE( LoadConfig(flat_config) == "flat" );
}

} // namespace common
} // namespace core
} // namespace black_library
