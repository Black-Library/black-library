#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <BlackLibraryDB.h>
#include <ParserDbAdapter.h>

#include "../CommonTestUtils.h"
#include "ParserTestUtils.h"

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

TEST_CASE( "Generic version check tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();
    auto md5_map = GenerateFullTestMd5Map();
    // std::vector<uint8_t> md5_int_vec{0, 1, 2, 3, 4, 5};
    // std::vector<BlackLibraryCommon::Md5Sum> md5_sums;
    // for (const auto & md5_int : md5_int_vec)
    // {
    //     md5_sums.emplace_back(md5_map.find(md5_int));
    // }
    // auto offset = GenerateVersionOffset(md5_sums);

    // REQUIRE( offset == 5 );
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'new section' version check test", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();

    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);

    for (const auto & md5 : md5_map)
    {
        REQUIRE( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    auto md5s = blacklibrary_db->GetMd5SumsFromUUID(RR_DUMMY_UUID);
    size_t i = 0;
    for (const auto & md5 : md5s)
    {
        REQUIRE( md5.second.uuid == RR_DUMMY_UUID );
        ++i;
    }
    REQUIRE( i == 6 );

    for (const auto & md5 : md5_map)
    {
        auto db_md5 = blacklibrary_db->ReadMd5SumUrl(md5.second.uuid, md5.second.url);
        REQUIRE(blacklibrary_db->DoesMd5SumExistUrl(md5.second.uuid, md5.second.url) == true);
        REQUIRE(blacklibrary_db->DoesWorkEntryUUIDExist(md5.second.uuid));
        auto db_adapter_md5 = db_adapter->ReadMd5(md5.second.uuid, md5.second.url);

        REQUIRE( db_md5.uuid == db_adapter_md5.uuid );
        REQUIRE( db_md5.url == db_adapter_md5.url );
        REQUIRE( db_md5.index_num == db_adapter_md5.index_num );
        REQUIRE( db_md5.md5_sum == db_adapter_md5.md5_sum );

        auto md5_check = db_adapter->CheckForMd5(md5.second.md5_sum, md5.second.uuid);
        REQUIRE( md5_check.uuid == RR_DUMMY_UUID );
        REQUIRE( md5_check.md5_sum != BlackLibraryCommon::EmptyMD5Version );
    }

    for (size_t i = 0; i < 6; ++i)
    {
        REQUIRE( md5_map.count(i) == 1 );
        auto md5 = md5_map.find(i);
        char dest_0[24];
        snprintf(dest_0, sizeof(dest_0), "dummy content %d to hash", md5->first);
        auto version_check = db_adapter->CheckVersion(std::string(dest_0), md5->second.uuid, md5->second.index_num, md5->second.date, md5->second.url);
        REQUIRE( version_check.already_exists == true );
    }

    auto version_check = db_adapter->CheckVersion("dummy content 6 to hash", RR_DUMMY_UUID, 6, 0, RR_URL_6);

    REQUIRE( version_check.has_error == false );
    REQUIRE( version_check.already_exists == false );
    REQUIRE( version_check.offset == 0 );

    md5s = blacklibrary_db->GetMd5SumsFromUUID(RR_DUMMY_UUID);
    i = 0;
    for (const auto & md5 : md5s)
    {
        REQUIRE( md5.second.uuid == RR_DUMMY_UUID );
        ++i;
    }
    REQUIRE( i == 7 );

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

TEST_CASE( "Clean up in case of failure" )
{
    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
