/**
 * log_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <LogOperations.h>

namespace black_library {

namespace core {

namespace common {

TEST_CASE( "InitRotatingLogger generic tests (pass)", "[single-file]" )
{
    REQUIRE ( InitRotatingLogger("test-logger-0", "/tmp/", true) == 0 );
    REQUIRE ( InitRotatingLogger("test-logger-0", "/tmp/", true) == 0 );
    REQUIRE ( InitRotatingLogger("test-logger-1", "/", true) == -1 );
    RemovePath("/tmp/test-logger-0.txt");
}

} // namespace common
} // namespace core
} // namespace black_library
