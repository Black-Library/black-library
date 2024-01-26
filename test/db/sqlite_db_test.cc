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
    SQLiteDB db(DefaultTestDBPath, "1.0");
    REQUIRE ( db.IsReady() == true );
}

TEST_CASE( "Test create database sqlite already exists (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath, "1.0");
}

TEST_CASE( "Test CRUD for entries sqlite (pass)", "[single-file]" )
{
    DBEntry work_entry = GenerateTestWorkEntry();
    DBErrorEntry error_entry = GenerateTestErrorEntry();

    SQLiteDB db(DefaultTestDBPath, "1.0");

    REQUIRE ( db.CreateEntry(work_entry) == 0 );
    REQUIRE ( db.CreateErrorEntry(error_entry) == 0 );
    REQUIRE ( db.DoesEntryUrlExist(work_entry.url).result == true );
    REQUIRE ( db.DoesEntryUUIDExist(work_entry.uuid).result == true );
    REQUIRE ( db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num).result == true );

    DBEntry work_read = db.ReadEntry(work_entry.uuid);
    REQUIRE ( work_read.uuid == work_entry.uuid );

    work_entry.author = "renamed-author";
    REQUIRE ( db.UpdateEntry(work_entry) == 0 );
    DBEntry work_update = db.ReadEntry(work_entry.uuid);
    REQUIRE ( work_update.author == work_entry.author );

    REQUIRE ( db.DeleteEntry(work_entry.uuid) == 0 );
    REQUIRE ( db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == 0 );
    REQUIRE ( db.DoesEntryUrlExist(work_entry.url).result == false );
    REQUIRE ( db.DoesEntryUUIDExist(work_entry.uuid).result == false );
    REQUIRE ( db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num).result == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for md5 checksum table sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath, "1.0");

    BlackLibraryCommon::Md5Sum md5 = GenerateTestMd5Sum();

    REQUIRE ( db.CreateMd5Sum(md5) == 0 );
    REQUIRE ( db.DoesMd5SumExistByIndexNum(md5.uuid, md5.index_num).result == true );
    REQUIRE ( db.DoesMd5SumExistBySecId(md5.uuid, md5.sec_id).result == true );
    REQUIRE ( db.DoesMd5SumExistBySeqNum(md5.uuid, md5.seq_num).result == true );

    BlackLibraryCommon::Md5Sum md5_read = db.ReadMd5SumByIndexNum(md5.uuid, md5.index_num);
    REQUIRE ( md5_read.uuid == md5.uuid );

    md5_read = db.ReadMd5SumBySecId(md5.uuid, md5.sec_id);
    REQUIRE ( md5_read.uuid == md5.uuid );
    REQUIRE ( md5_read.index_num == md5.index_num );
    REQUIRE ( md5_read.md5_sum == md5.md5_sum );
    REQUIRE ( md5_read.date == md5.date );
    REQUIRE ( md5_read.sec_id == md5.sec_id );
    REQUIRE ( md5_read.seq_num == md5.seq_num );
    REQUIRE ( md5_read.version_num == md5.version_num );

    md5_read = db.ReadMd5SumBySeqNum(md5.uuid, md5.seq_num);
    REQUIRE ( md5_read.uuid == md5.uuid );
    REQUIRE ( md5_read.index_num == md5.index_num );
    REQUIRE ( md5_read.md5_sum == md5.md5_sum );
    REQUIRE ( md5_read.date == md5.date );
    REQUIRE ( md5_read.sec_id == md5.sec_id );
    REQUIRE ( md5_read.seq_num == md5.seq_num );
    REQUIRE ( md5_read.version_num == md5.version_num );

    size_t version_num_0 = db.GetVersionFromMd5(md5.uuid, md5.index_num);
    REQUIRE ( version_num_0 == md5.version_num );

    size_t version_num_1 = db.GetVersionFromMd5("", 8);
    REQUIRE ( version_num_1 == 0 );

    BlackLibraryCommon::Md5Sum new_md5 = GenerateTestMd5Sum();

    new_md5.md5_sum = "17e8f0b4718aa78060a067fcee68513c";
    new_md5.date = 101;
    new_md5.version_num = 5;
    new_md5.sec_id = "new-md5-sec-id";
    REQUIRE ( db.UpdateMd5SumByIndexNum(new_md5) == 0 );
    BlackLibraryCommon::Md5Sum md5_update = db.ReadMd5SumBySecId(new_md5.uuid, new_md5.sec_id);
    REQUIRE ( md5_update.uuid == md5.uuid );
    REQUIRE ( md5_update.index_num == 18 );
    REQUIRE ( md5_update.md5_sum == "17e8f0b4718aa78060a067fcee68513c" );
    REQUIRE ( md5_update.date == 101 );
    REQUIRE ( md5_update.sec_id == "new-md5-sec-id" );

    REQUIRE ( db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    md5.index_num = 1000;
    REQUIRE ( db.CreateMd5Sum(md5) == 0 );
    BlackLibraryCommon::Md5Sum md5_update_index_num = db.ReadMd5SumBySeqNum(md5.uuid, md5.seq_num);

    REQUIRE ( md5_update_index_num.md5_sum == md5.md5_sum );
    REQUIRE ( md5_update_index_num.date == md5.date );
    REQUIRE ( md5_update_index_num.version_num == md5.version_num );
    REQUIRE ( md5_update_index_num.sec_id == md5.sec_id );
    REQUIRE ( md5_update_index_num.seq_num == md5.seq_num );
    REQUIRE ( md5_update_index_num.index_num == md5.index_num );

    REQUIRE ( db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    REQUIRE ( db.DoesMd5SumExistByIndexNum(md5.uuid, md5.index_num).result == false );
    REQUIRE ( db.DoesMd5SumExistBySecId(md5.uuid, md5.sec_id).result == false );
    REQUIRE ( db.DoesMd5SumExistBySeqNum(md5.uuid, md5.seq_num).result == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test reading md5s back ordered by index_num sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath, "1.0");

    BlackLibraryCommon::Md5Sum md5_0 = GenerateTestMd5Sum();
    md5_0.sec_id = "md5-sec-id-0";

    REQUIRE ( db.CreateMd5Sum(md5_0) == 0 );
    REQUIRE ( db.DoesMd5SumExistByIndexNum(md5_0.uuid, md5_0.index_num).result == true );
    REQUIRE ( db.DoesMd5SumExistBySecId(md5_0.uuid, md5_0.sec_id).result == true );
    REQUIRE ( db.DoesMd5SumExistBySeqNum(md5_0.uuid, md5_0.seq_num).result == true );

    BlackLibraryCommon::Md5Sum md5_1 = GenerateTestMd5Sum();
    BlackLibraryCommon::Md5Sum md5_2 = GenerateTestMd5Sum();
    md5_1.md5_sum = DefaultTestMd5_1;
    md5_2.md5_sum = DefaultTestMd5_2;
    md5_1.index_num = 20;
    md5_2.index_num = 10;
    md5_1.sec_id = "md5-sec-id-1";
    md5_2.sec_id = "md5-sec-id-2";

    REQUIRE ( db.CreateMd5Sum(md5_1) == 0 );
    REQUIRE ( db.CreateMd5Sum(md5_2) == 0 );

    REQUIRE ( db.DoesMd5SumExistByIndexNum(md5_1.uuid, md5_1.index_num).result == true );
    REQUIRE ( db.DoesMd5SumExistByIndexNum(md5_2.uuid, md5_2.index_num).result == true );

    std::vector<BlackLibraryCommon::Md5Sum> md5_sums = db.GetMd5SumsFromUUID(md5_0.uuid);

    size_t lowest = md5_2.index_num;
    for (const auto & md5 : md5_sums)
    {
        REQUIRE ( lowest <= md5.index_num );
        lowest = md5.index_num;
    }

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test reading md5s back max seq num sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath, "1.0");

    DBEntry work_entry = GenerateTestWorkEntry();
    REQUIRE ( db.CreateEntry(work_entry) == 0 );

    for (size_t i = 0; i < 10; ++i)
    {
        BlackLibraryCommon::Md5Sum md5_add = GenerateTestMd5Sum();
        md5_add.md5_sum = BlackLibraryCommon::GetMD5Hash(std::to_string(i));
        md5_add.index_num = i;
        md5_add.seq_num = i;
        if ( i >= 5 )
        {
            md5_add.seq_num = BlackLibraryCommon::MaxSeqNum;
        }
        REQUIRE ( db.CreateMd5Sum(md5_add) == 0 );
        REQUIRE ( db.DoesMd5SumExistByIndexNum(md5_add.uuid, i).result == true );
    }

    std::vector<BlackLibraryCommon::Md5Sum> md5_sums = db.GetMd5SumsFromUUID(DefaultTestUUID);

    REQUIRE ( md5_sums.size() == 10 );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test basic func for refresh table sqlite (pass)", "[single-file]" )
{
    SQLiteDB db(DefaultTestDBPath, "1.0");

    DBRefresh refresh = GenerateTestRefresh();

    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == false );
    REQUIRE ( db.DoesMinRefreshExist().result == false );
    REQUIRE ( db.CreateRefresh(refresh) == 0 );
    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == true );
    REQUIRE ( db.DoesMinRefreshExist().result == true );

    auto read_refresh = db.ReadRefresh(refresh.uuid);
    REQUIRE ( read_refresh.uuid == refresh.uuid );
    REQUIRE ( read_refresh.refresh_date == refresh.refresh_date );

    auto next_refresh = db.GetRefreshFromMinDate();

    REQUIRE ( next_refresh.uuid == refresh.uuid );
    REQUIRE ( next_refresh.refresh_date == refresh.refresh_date );

    REQUIRE ( db.DeleteRefresh(refresh.uuid) == 0 );

    REQUIRE ( db.DoesRefreshExist(refresh.uuid).result == false );
    REQUIRE ( db.DoesMinRefreshExist().result == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test teardown tmp db" )
{
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

} // namespace db
} // namespace core
} // namespace black_library
