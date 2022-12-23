/**
 * sqlite_db_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <LogOperations.h>

#include <SQLiteDB.h>

#include <DBTestUtils.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Test init sqlite logger (pass)", "[single-file]")
{
    BlackLibraryCommon::InitRotatingLogger("db", "/tmp/", true);
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test setup database sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath);
    REQUIRE( db.IsReady() == true );
}

TEST_CASE( "Test create database sqlite already exists (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for entries sqlite (pass)", "[single-file]" )
{
    DBEntry staging_entry = GenerateTestStagingEntry();
    DBEntry black_entry = GenerateTestBlackEntry();
    DBErrorEntry error_entry = GenerateTestErrorEntry();

    SQLiteDB db(DefaultTestDBPath);

    REQUIRE( db.CreateEntry(staging_entry, STAGING_ENTRY) == 0 );
    REQUIRE( db.CreateEntry(black_entry, BLACK_ENTRY) == 0 );
    REQUIRE( db.CreateErrorEntry(error_entry) == 0 );
    REQUIRE( db.DoesEntryUrlExist(staging_entry.url, STAGING_ENTRY).result == true );
    REQUIRE( db.DoesEntryUUIDExist(staging_entry.uuid, STAGING_ENTRY).result == true );
    REQUIRE( db.DoesEntryUrlExist(black_entry.url, BLACK_ENTRY).result == true );
    REQUIRE( db.DoesEntryUUIDExist(black_entry.uuid, BLACK_ENTRY).result == true );
    REQUIRE( db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num).result == true );

    DBEntry staging_read = db.ReadEntry(staging_entry.uuid, STAGING_ENTRY);
    DBEntry black_read = db.ReadEntry(black_entry.uuid, BLACK_ENTRY);
    REQUIRE( staging_read.uuid == staging_entry.uuid );
    REQUIRE( black_read.uuid == black_entry.uuid );

    staging_entry.author = "renamed-author";
    black_entry.author = "renamed-author";
    REQUIRE( db.UpdateEntry(staging_entry, STAGING_ENTRY) == 0 );
    REQUIRE( db.UpdateEntry(black_entry, BLACK_ENTRY) == 0 );
    DBEntry staging_update = db.ReadEntry(staging_entry.uuid, STAGING_ENTRY);
    DBEntry black_update = db.ReadEntry(black_entry.uuid, BLACK_ENTRY);
    REQUIRE( staging_update.author == staging_entry.author );
    REQUIRE( black_update.author == staging_entry.author );

    REQUIRE( db.DeleteEntry(staging_entry.uuid, STAGING_ENTRY) == 0 );
    REQUIRE( db.DeleteEntry(black_entry.uuid, BLACK_ENTRY) == 0 );
    REQUIRE( db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == 0 );
    REQUIRE( db.DoesEntryUrlExist(staging_entry.url, STAGING_ENTRY).result == false );
    REQUIRE( db.DoesEntryUUIDExist(staging_entry.uuid, STAGING_ENTRY).result == false );
    REQUIRE( db.DoesEntryUrlExist(black_entry.url, BLACK_ENTRY).result == false );
    REQUIRE( db.DoesEntryUUIDExist(black_entry.uuid, BLACK_ENTRY).result == false );
    REQUIRE( db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num).result == false );
}

TEST_CASE( "Test CRUD for md5 checksum table sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath);

    DBMd5Sum md5 = GenerateTestMd5Sum();

    REQUIRE ( db.CreateMd5Sum(md5) == 0 );
    REQUIRE ( db.DoesMd5SumExist(md5.uuid, md5.index_num).result == true );

    DBMd5Sum md5_read = db.ReadMd5Sum(md5.uuid, md5.index_num);
    REQUIRE ( md5_read.uuid == md5.uuid );

    size_t version_num_0 = db.GetVersionFromMd5(md5.uuid, md5.index_num);
    REQUIRE ( version_num_0 == md5.version_num );

    size_t version_num_1 = db.GetVersionFromMd5("", 8);
    REQUIRE ( version_num_1 == 0 );

    md5.md5_sum = "17e8f0b4718aa78060a067fcee68513c";
    REQUIRE ( db.UpdateMd5Sum(md5) == 0 );
    DBMd5Sum md5_update = db.ReadMd5Sum(md5.uuid, md5.index_num);
    REQUIRE( md5_update.md5_sum == md5.md5_sum );

    REQUIRE ( db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    REQUIRE ( db.DoesMd5SumExist(md5.uuid, md5.index_num).result == false );
}

TEST_CASE( "Test basic func for refresh table sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath);

    DBRefresh refresh = GenerateTestRefresh();

    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == false );
    REQUIRE ( db.DoesMinRefreshExist().result == false );
    REQUIRE ( db.CreateRefresh(refresh) == 0 );
    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == true );
    REQUIRE ( db.DoesMinRefreshExist().result == true );

    auto read_refresh = db.ReadRefresh(refresh.uuid);
    REQUIRE( read_refresh.uuid == refresh.uuid );
    REQUIRE( read_refresh.refresh_date == refresh.refresh_date );

    auto next_refresh = db.GetRefreshFromMinDate();

    REQUIRE ( next_refresh.uuid == refresh.uuid );
    REQUIRE ( next_refresh.refresh_date == refresh.refresh_date );

    REQUIRE ( db.DeleteRefresh(refresh.uuid) == 0 );

    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == false );
    REQUIRE ( db.DoesMinRefreshExist().result == false );
}

} // namespace db
} // namespace core
} // namespace black_library
