#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <Parser.h>
#include <ParserRR.h>

#include <CommonTestUtils.h>

namespace black_library {

namespace core {

namespace parsers {


namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Generic parser tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();
    Parser parser(parser_t::ERROR_PARSER, config);
    parser.SetLocalFilePath("foo");
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "RR parser tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();
    RR::ParserRR parser(config);
    parser.SetLocalFilePath("foo");
    REQUIRE ( parser.GetParserType() == parser_t::RR_PARSER );
    REQUIRE ( parser.GetSourceUrl() == BlackLibraryCommon::RR::source_url );
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
