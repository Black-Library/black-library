/**
 * source_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <SourceInformation.h>

namespace black_library {

namespace core {

namespace common {

TEST_CASE( "GenerateUrlFromSourceUrl generic tests (pass)", "[single-file]" )
{
    REQUIRE( GenerateUrlFromSourceUrl("") == "https://" );
    REQUIRE( GenerateUrlFromSourceUrl(ERROR::source_url) == "https://" );
    REQUIRE( GenerateUrlFromSourceUrl(AO3::source_url) == "https://archiveofourown.org" );
    REQUIRE( GenerateUrlFromSourceUrl(FFN::source_url) == "https://www.fanfiction.net" );
    REQUIRE( GenerateUrlFromSourceUrl(RR::source_url) == "https://www.royalroad.com" );
    REQUIRE( GenerateUrlFromSourceUrl(SBF::source_url) == "https://forums.spacebattles.com" );
    REQUIRE( GenerateUrlFromSourceUrl(SVF::source_url) == "https://forums.sufficientvelocity.com" );
    REQUIRE( GenerateUrlFromSourceUrl(YT::source_url) == "https://youtube.com" );
}

TEST_CASE( "SourceInformationMember generic tests (pass)", "[single-file]" )
{
}

} // namespace common
} // namespace core
} // namespace black_library
