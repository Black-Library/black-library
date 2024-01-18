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

TEST_CASE( "Generic ParserIndexEntry tests (pass)", "[single-file]" )
{
    ParserIndexEntry index_entry_0;
    ParserIndexEntry index_entry_1;

    index_entry_0.data_url = RR_URL_0;
    index_entry_1.data_url = RR_URL_1;

    std::priority_queue<ParserIndexEntry, std::vector<ParserIndexEntry>, ParserIndexEntryGreaterThanByIdentifier> index_entry_identifier_queue;

    index_entry_identifier_queue.push(index_entry_0);
    index_entry_identifier_queue.push(index_entry_1);

    ParserIndexEntry index_entry_check = index_entry_identifier_queue.top();
    std::string index_entry_identifier_str = BlackLibraryCommon::GetWorkChapterIdentifierFromUrl(index_entry_check.data_url);
    REQUIRE ( index_entry_identifier_str == "1" );
}

TEST_CASE( "Generic parser tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();
    TestIndexEntryParser test_index_entry_parser(config);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic CalculateIndexBounds test", "[single-file]" )
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

    ParserJob parser_job;
    parser_job.uuid = test_entry.uuid;
    parser_job.url = test_entry.url;
    parser_job.last_url = test_entry.last_url;
    parser_job.start_number = 1;
    parser_job.end_number = 0;

    test_index_entry_parser.TestCalculateIndexBounds(parser_job);

    REQUIRE ( test_index_entry_parser.GetIndex() == 0 );
    REQUIRE ( test_index_entry_parser.GetTargetStartIndex() == 0 );
    REQUIRE ( test_index_entry_parser.GetTargetEndIndex() == std::numeric_limits<size_t>::max() );

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
