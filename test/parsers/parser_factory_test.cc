#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <ParserFactory.h>

#include "../CommonTestUtils.h"

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
    parser_factory.GetParserByType(parser_t::RR_PARSER);
    parser_factory.GetParserByType(parser_t::SBF_PARSER);
    parser_factory.GetParserByType(parser_t::SVF_PARSER);
    parser_factory.GetParserByType(parser_t::WP_PARSER);

    parser_factory.GetParserByUrl(AO3_DUMMY_URL);
    parser_factory.GetParserByUrl(RR_DUMMY_URL);
    parser_factory.GetParserByUrl(SBF_DUMMY_URL);
    parser_factory.GetParserByUrl(SVF_DUMMY_URL);
    parser_factory.GetParserByUrl(WP_DUMMY_URL);

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
