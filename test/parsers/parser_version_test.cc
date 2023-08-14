#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <ParserTestUtils.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Generic version check tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();
    auto md5_map = GenerateFullTestMd5Map();
    std::vector<uint8_t> md5_int_vec{0, 1, 2, 3, 4, 5};
    std::vector<BlackLibraryCommon::Md5Sum> md5_sums;
    for (const auto & md5_int : md5_int_vec)
    {
        md5_sums.emplace_back(md5_map.find(md5_int));
    }
    auto offset = GenerateVersionOffset(md5_sums);
    REQUIRE( offset == 5 );
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'new section' version check test", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'middle gap' version check test", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'start missing' version check test", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
