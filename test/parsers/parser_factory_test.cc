#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <ParserFactory.h>

#include <CommonTestUtils.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryParsers = black_library::core::parsers;

TEST_CASE( "Generic parser factory tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    njson config = GenerateParserTestConfig();
    ParserFactory parser_factory(config, nullptr);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Parser factory url tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    njson config = GenerateParserTestConfig();
    ParserFactory parser_factory(config, nullptr);

    parser_factory.GetParserByType(parser_t::AO3_PARSER);
    auto factory_result_ao3 = parser_factory.GetParserByUrl(AO3_DUMMY_URL);
    REQUIRE ( factory_result_ao3.has_error == false );
    REQUIRE ( factory_result_ao3.parser_result->GetParserType() == parser_t::AO3_PARSER );

    parser_factory.GetParserByType(parser_t::RR_PARSER);
    auto factory_result_rr  = parser_factory.GetParserByUrl(RR_DUMMY_URL);
    REQUIRE ( factory_result_rr.has_error == false );
    REQUIRE ( factory_result_rr.parser_result->GetParserType() == parser_t::RR_PARSER );

    parser_factory.GetParserByType(parser_t::SBF_PARSER);
    auto factory_result_sbf = parser_factory.GetParserByUrl(SBF_DUMMY_URL);
    REQUIRE ( factory_result_sbf.has_error == false );
    REQUIRE ( factory_result_sbf.parser_result->GetParserType() == parser_t::SBF_PARSER );

    parser_factory.GetParserByType(parser_t::SVF_PARSER);
    auto factory_result_svf = parser_factory.GetParserByUrl(SVF_DUMMY_URL);
    REQUIRE ( factory_result_svf.has_error == false );
    REQUIRE ( factory_result_svf.parser_result->GetParserType() == parser_t::SVF_PARSER );

    parser_factory.GetParserByType(parser_t::WP_PARSER);
    auto factory_result_wp  = parser_factory.GetParserByUrl(WP_DUMMY_URL);
    REQUIRE ( factory_result_wp.has_error == false );
    REQUIRE ( factory_result_wp.parser_result->GetParserType() == parser_t::WP_PARSER );

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
