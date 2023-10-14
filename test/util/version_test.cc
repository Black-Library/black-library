#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <VersionOperations.h>

#include "../CommonTestUtils.h"

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Generic version check tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    auto content_md5_0 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_0);
    REQUIRE( content_md5_0 == RR_MD5_0 );

    auto content_md5_1 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_1);
    REQUIRE( content_md5_1 == RR_MD5_1 );

    auto content_md5_2 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_2);
    REQUIRE( content_md5_2 == RR_MD5_2 );

    auto content_md5_3 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_3);
    REQUIRE( content_md5_3 == RR_MD5_3 );

    auto content_md5_4 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_4);
    REQUIRE( content_md5_4 == RR_MD5_4 );

    auto content_md5_5 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_5);
    REQUIRE( content_md5_5 == RR_MD5_5 );

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Clean up in case of failure" )
{
    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
