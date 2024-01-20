#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <BlackLibraryDB.h>
#include <ParserDbAdapter.h>

#include <TestIndexEntryParser.h>

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

    // setup db and adapter
    auto config = GenerateParserTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    // setup md5
    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();
    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);
    for (const auto & md5 : md5_map)
    {
        REQUIRE ( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    // check to make sure md5s are in the table
    auto md5s = blacklibrary_db->GetMd5SumsFromUUIDSeqNum(RR_DUMMY_UUID);
    REQUIRE ( md5s.size() == 6 );

    for (const auto & md5 : md5_map)
    {
        auto db_md5 = blacklibrary_db->ReadMd5SumBySeqNum(md5.second.uuid, md5.second.seq_num);
        REQUIRE (blacklibrary_db->DoesMd5SumExistBySecId(md5.second.uuid, md5.second.sec_id) == true);
        REQUIRE (blacklibrary_db->DoesMd5SumExistBySeqNum(md5.second.uuid, md5.second.seq_num) == true);
        REQUIRE (blacklibrary_db->DoesWorkEntryUUIDExist(md5.second.uuid));

        auto db_adapter_md5 = db_adapter->ReadMd5BySeqNum(md5.second.uuid, md5.second.seq_num);
        REQUIRE ( db_md5.uuid == db_adapter_md5.uuid );
        REQUIRE ( db_md5.sec_id == db_adapter_md5.sec_id );
        REQUIRE ( db_md5.seq_num == db_adapter_md5.seq_num );
        REQUIRE ( db_md5.index_num == db_adapter_md5.index_num );
        REQUIRE ( db_md5.md5_sum == db_adapter_md5.md5_sum );

        auto md5_check = db_adapter->CheckForMd5(md5.second.md5_sum, md5.second.uuid);
        REQUIRE ( md5_check.uuid == RR_DUMMY_UUID );
        REQUIRE ( md5_check.md5_sum != BlackLibraryCommon::EmptyMD5Version );
    }

    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic already exists version check tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);

    // setup db and adapter
    auto config = GenerateParserTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    // setup md5
    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();
    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);
    for (const auto & md5 : md5_map)
    {
        REQUIRE ( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    for (size_t i = 0; i < 6; ++i)
    {
        REQUIRE ( md5_map.count(i) == 1 );
        auto md5 = md5_map.find(i);
        char dest_0[24];
        snprintf(dest_0, sizeof(dest_0), "dummy content %d to hash", md5->first);
        auto version_check = db_adapter->CheckVersion(std::string(dest_0), md5->second.uuid, md5->second.index_num, md5->second.date);
        REQUIRE ( version_check.already_exists == true );
    }

    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'new section' version check test", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);

    // setup db and adapter
    auto config = GenerateParserTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    // setup md5
    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();
    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);
    for (const auto & md5 : md5_map)
    {
        REQUIRE ( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    TestIndexEntryParser test_index_entry_parser(config);
    test_index_entry_parser.SetDbAdapter(db_adapter);

    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'middle gap' version check test", "[single-file]" )
{
    // setup db and adapter
    auto config = GenerateParserTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    // setup md5
    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();
    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);
    for (const auto & md5 : md5_map)
    {
        REQUIRE ( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic 'start missing' version check test", "[single-file]" )
{
    // setup db and adapter
    auto config = GenerateParserTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);
    auto db_adapter = std::make_shared<ParserDbAdapter>(config, blacklibrary_db);

    // setup md5
    auto md5_map = GenerateFullTestMd5Map();
    auto test_entry = GenerateTestWorkEntry();
    REQUIRE (blacklibrary_db->CreateWorkEntry(test_entry) == 0);
    for (const auto & md5 : md5_map)
    {
        REQUIRE ( blacklibrary_db->CreateMd5Sum(md5.second) == 0 );
    }

    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
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
