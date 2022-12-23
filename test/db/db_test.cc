/**
 * db_test.cc
 */

#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <LogOperations.h>

#include <BlackLibraryDB.h>

#include <DBTestUtils.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Test init black library db logger (pass)", "[single-file]")
{
    BlackLibraryCommon::InitRotatingLogger("db", "/tmp/", false);
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test setup black library db (pass)", "[single-file]" )
{
    auto config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE( blacklibrary_db.IsReady() == true );
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for empty entries black library (pass)", "[single-file]" )
{
    DBEntry staging_entry;
    DBEntry black_entry;
    DBErrorEntry error_entry;

    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE( blacklibrary_db.CreateStagingEntry(staging_entry) == -1 );
    REQUIRE( blacklibrary_db.CreateBlackEntry(black_entry) == -1 );
    REQUIRE( blacklibrary_db.CreateErrorEntry(error_entry) == -1 );

    DBEntry staging_read = blacklibrary_db.ReadStagingEntry(staging_entry.uuid);
    DBEntry black_read = blacklibrary_db.ReadBlackEntry(black_entry.uuid);
    REQUIRE( staging_read.uuid == "" );
    REQUIRE( black_read.uuid == "" );

    REQUIRE( blacklibrary_db.UpdateStagingEntry(staging_entry) == -1 );
    REQUIRE( blacklibrary_db.UpdateBlackEntry(black_entry) == -1 );

    REQUIRE( blacklibrary_db.DeleteStagingEntry(staging_entry.uuid) == -1 );
    REQUIRE( blacklibrary_db.DeleteBlackEntry(black_entry.uuid) == -1 );
    REQUIRE( blacklibrary_db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == -1 );
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for staging, black, and error entry tables black library (pass)", "[single-file]" )
{
    DBEntry staging_entry = GenerateTestStagingEntry();
    DBEntry black_entry = GenerateTestBlackEntry();
    DBErrorEntry error_entry = GenerateTestErrorEntry();

    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE( blacklibrary_db.CreateStagingEntry(staging_entry) == 0 );
    REQUIRE( blacklibrary_db.CreateBlackEntry(black_entry) == 0 );
    REQUIRE( blacklibrary_db.CreateErrorEntry(error_entry) == 0 );
    REQUIRE( blacklibrary_db.DoesStagingEntryUrlExist(staging_entry.url) == true );
    REQUIRE( blacklibrary_db.DoesStagingEntryUUIDExist(staging_entry.uuid) == true );
    REQUIRE( blacklibrary_db.DoesBlackEntryUrlExist(black_entry.url) == true );
    REQUIRE( blacklibrary_db.DoesBlackEntryUUIDExist(black_entry.uuid) == true );
    REQUIRE( blacklibrary_db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num) == true );

    DBEntry staging_read = blacklibrary_db.ReadStagingEntry(staging_entry.uuid);
    DBEntry black_read = blacklibrary_db.ReadBlackEntry(black_entry.uuid);
    REQUIRE( staging_read.uuid == staging_entry.uuid );
    REQUIRE( black_read.uuid == black_entry.uuid );

    staging_entry.author = "renamed-author";
    black_entry.author = "renamed-author";
    REQUIRE( blacklibrary_db.UpdateStagingEntry(staging_entry) == 0 );
    REQUIRE( blacklibrary_db.UpdateBlackEntry(black_entry) == 0 );
    DBEntry staging_update = blacklibrary_db.ReadStagingEntry(staging_entry.uuid);
    DBEntry black_update = blacklibrary_db.ReadBlackEntry(black_entry.uuid);
    REQUIRE( staging_update.author == staging_entry.author );
    REQUIRE( black_update.author == black_entry.author );

    REQUIRE( blacklibrary_db.DeleteStagingEntry(staging_entry.uuid) == 0 );
    REQUIRE( blacklibrary_db.DeleteBlackEntry(black_entry.uuid) == 0 );
    REQUIRE( blacklibrary_db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == 0 );
    REQUIRE( blacklibrary_db.DoesStagingEntryUrlExist(staging_entry.url) == false );
    REQUIRE( blacklibrary_db.DoesStagingEntryUUIDExist(staging_entry.uuid) == false );
    REQUIRE( blacklibrary_db.DoesBlackEntryUrlExist(black_entry.url) == false );
    REQUIRE( blacklibrary_db.DoesBlackEntryUUIDExist(black_entry.uuid) == false );
    REQUIRE( blacklibrary_db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num) == false );
}

TEST_CASE( "Test CRUD for md5 checksum table black library (pass)", "[single-file]" )
{
    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    DBMd5Sum md5 = GenerateTestMd5Sum();

    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5) == 0 );
    REQUIRE ( blacklibrary_db.DoesMd5SumExist(md5.uuid, md5.index_num) == true );

    DBMd5Sum md5_read = blacklibrary_db.ReadMd5Sum(md5.uuid, md5.index_num);
    REQUIRE ( md5_read.uuid == md5.uuid );

    size_t version_num_0 = blacklibrary_db.GetVersionFromMd5(md5.uuid, md5.index_num);
    REQUIRE ( version_num_0 == md5.version_num );

    size_t version_num_1 = blacklibrary_db.GetVersionFromMd5("", 8);
    REQUIRE ( version_num_1 == 0 );

    md5.md5_sum = "17e8f0b4718aa78060a067fcee68513c";
    REQUIRE ( blacklibrary_db.UpdateMd5Sum(md5) == 0 );
    DBMd5Sum md5_update = blacklibrary_db.ReadMd5Sum(md5.uuid, md5.index_num);
    REQUIRE( md5_update.md5_sum == md5.md5_sum );

    REQUIRE ( blacklibrary_db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    REQUIRE ( blacklibrary_db.DoesMd5SumExist(md5.uuid, md5.index_num) == false );
}

TEST_CASE( "Test basic func for refresh table sqlite (pass)", "[single-file]" )
{
    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    DBRefresh refresh = GenerateTestRefresh();

    REQUIRE ( blacklibrary_db.DoesRefreshExist(refresh.uuid) == false );
    REQUIRE ( blacklibrary_db.DoesMinRefreshExist() == false );
    REQUIRE ( blacklibrary_db.CreateRefresh(refresh) == 0 );
    REQUIRE ( blacklibrary_db.DoesRefreshExist(refresh.uuid) == true );
    REQUIRE ( blacklibrary_db.DoesMinRefreshExist() == true );

    auto read_refresh = blacklibrary_db.ReadRefresh(refresh.uuid);
    REQUIRE( read_refresh.uuid == refresh.uuid );
    REQUIRE( read_refresh.refresh_date == refresh.refresh_date );

    auto next_refresh = blacklibrary_db.GetRefreshFromMinDate();

    REQUIRE ( next_refresh.uuid == refresh.uuid );
    REQUIRE ( next_refresh.refresh_date == refresh.refresh_date );

    REQUIRE ( blacklibrary_db.DeleteRefresh(refresh.uuid) == 0 );

    REQUIRE ( blacklibrary_db.DoesRefreshExist(refresh.uuid) == false );
    REQUIRE ( blacklibrary_db.DoesMinRefreshExist() == false );
}

} // namespace db
} // namespace core
} // namespace black_library
