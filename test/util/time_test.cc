/**
 * time_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <TimeOperations.h>

namespace black_library {

namespace core {

namespace common {

TEST_CASE( "GetGUITimeString generic tests (pass)", "[single-file]" )
{
    std::time_t test_time_0 = ParseTimet("01/02/2003 7:08:09 PM", "%m/%d/%Y %I:%M:%S %p");
    // 2003-01-02 19:08 EST to GMT is 2003-01-03 00:08
    std::string gui_str = GetGUITimeString(test_time_0);
    REQUIRE ( gui_str == std::string("2003-01-03 00:08") );
}

// TEST_CASE( "GetISOTimeString generic tests (pass)", "[single-file]" )
// {
//     REQUIRE ( GetISOTimeString(test_time_0) == "" );
// }

TEST_CASE( "GetUnixTime generic tests (pass)", "[single-file]" )
{
    REQUIRE ( GetUnixTime() );
}

TEST_CASE( "ParseTimet generic tests (pass)", "[single-file]" )
{
    time_t test_time_0 = 1540762483 + 5 * 60 * 60; // 5 hour modifier for EST -> GMT

    time_t parse_time_rr = ParseTimet("10/28/2018 9:34:43 PM +00:00", "%m/%d/%Y %I:%M:%S %p");
    
    REQUIRE ( parse_time_rr == test_time_0 );
}


} // namespace common
} // namespace core
} // namespace black_library
