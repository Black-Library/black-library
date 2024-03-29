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
    BlackLibraryCommon::InitRotatingLogger("db", "/tmp/log", false);
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test setup black library db (pass)", "[single-file]" )
{
    auto config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE ( blacklibrary_db.IsReady() == true );
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for empty entries black library (pass)", "[single-file]" )
{
    DBEntry work_entry;
    DBErrorEntry error_entry;

    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE ( blacklibrary_db.CreateWorkEntry(work_entry) == -1 );
    REQUIRE ( blacklibrary_db.CreateErrorEntry(error_entry) == -1 );

    DBEntry work_read = blacklibrary_db.ReadWorkEntry(work_entry.uuid);
    REQUIRE ( work_read.uuid == "" );

    REQUIRE ( blacklibrary_db.UpdateWorkEntry(work_entry) == -1 );

    REQUIRE ( blacklibrary_db.DeleteWorkEntry(work_entry.uuid) == -1 );
    REQUIRE ( blacklibrary_db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == -1 );
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for work and error entry tables black library (pass)", "[single-file]" )
{
    DBEntry work_entry = GenerateTestWorkEntry();
    DBErrorEntry error_entry = GenerateTestErrorEntry();

    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    REQUIRE ( blacklibrary_db.CreateWorkEntry(work_entry) == 0 );
    REQUIRE ( blacklibrary_db.CreateErrorEntry(error_entry) == 0 );
    REQUIRE ( blacklibrary_db.DoesWorkEntryUrlExist(work_entry.url) == true );
    REQUIRE ( blacklibrary_db.DoesWorkEntryUUIDExist(work_entry.uuid) == true );
    REQUIRE ( blacklibrary_db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num) == true );

    DBEntry work_read = blacklibrary_db.ReadWorkEntry(work_entry.uuid);
    REQUIRE ( work_read.uuid == work_entry.uuid );

    work_entry.author = "renamed-author";
    REQUIRE ( blacklibrary_db.UpdateWorkEntry(work_entry) == 0 );
    DBEntry work_update = blacklibrary_db.ReadWorkEntry(work_entry.uuid);
    REQUIRE ( work_update.author == work_entry.author );

    REQUIRE ( blacklibrary_db.DeleteWorkEntry(work_entry.uuid) == 0 );
    REQUIRE ( blacklibrary_db.DeleteErrorEntry(error_entry.uuid, error_entry.progress_num) == 0 );
    REQUIRE ( blacklibrary_db.DoesWorkEntryUrlExist(work_entry.url) == false );
    REQUIRE ( blacklibrary_db.DoesWorkEntryUUIDExist(work_entry.uuid) == false );
    REQUIRE ( blacklibrary_db.DoesErrorEntryExist(error_entry.uuid, error_entry.progress_num) == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test CRUD for md5 checksum table black library (pass)", "[single-file]" )
{
    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    BlackLibraryCommon::Md5Sum md5 = GenerateTestMd5Sum();

    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5) == 0 );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistByIndexNum(md5.uuid, md5.index_num) == true );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistBySecId(md5.uuid, md5.sec_id) == true );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistBySeqNum(md5.uuid, md5.seq_num) == true );

    BlackLibraryCommon::Md5Sum md5_read = blacklibrary_db.ReadMd5SumByIndexNum(md5.uuid, md5.index_num);
    REQUIRE ( md5_read.uuid == md5.uuid );
    REQUIRE ( md5_read.index_num == md5.index_num );
    REQUIRE ( md5_read.md5_sum == md5.md5_sum );
    REQUIRE ( md5_read.date == md5.date );
    REQUIRE ( md5_read.sec_id == md5.sec_id );
    REQUIRE ( md5_read.seq_num == md5.seq_num );
    REQUIRE ( md5_read.version_num == md5.version_num );

    md5_read = blacklibrary_db.ReadMd5SumBySecId(md5.uuid, md5.sec_id);
    REQUIRE ( md5_read.uuid == md5.uuid );
    REQUIRE ( md5_read.index_num == md5.index_num );
    REQUIRE ( md5_read.md5_sum == md5.md5_sum );
    REQUIRE ( md5_read.date == md5.date );
    REQUIRE ( md5_read.sec_id == md5.sec_id );
    REQUIRE ( md5_read.seq_num == md5.seq_num );
    REQUIRE ( md5_read.version_num == md5.version_num );

    md5_read = blacklibrary_db.ReadMd5SumBySeqNum(md5.uuid, md5.seq_num);
    REQUIRE ( md5_read.uuid == md5.uuid );
    REQUIRE ( md5_read.index_num == md5.index_num );
    REQUIRE ( md5_read.md5_sum == md5.md5_sum );
    REQUIRE ( md5_read.date == md5.date );
    REQUIRE ( md5_read.sec_id == md5.sec_id );
    REQUIRE ( md5_read.seq_num == md5.seq_num );
    REQUIRE ( md5_read.version_num == md5.version_num );

    size_t version_num_0 = blacklibrary_db.GetVersionFromMd5(md5.uuid, md5.index_num);
    REQUIRE ( version_num_0 == md5.version_num );

    size_t version_num_1 = blacklibrary_db.GetVersionFromMd5("", 8);
    REQUIRE ( version_num_1 == 0 );

    BlackLibraryCommon::Md5Sum new_md5 = GenerateTestMd5Sum();

    new_md5.md5_sum = "17e8f0b4718aa78060a067fcee68513c";
    new_md5.date = 101;
    new_md5.version_num = 5;
    new_md5.sec_id = "new-md5-sec-id";
    REQUIRE ( blacklibrary_db.UpdateMd5SumByIndexNum(new_md5) == 0 );
    BlackLibraryCommon::Md5Sum md5_update = blacklibrary_db.ReadMd5SumBySecId(new_md5.uuid, new_md5.sec_id);
    REQUIRE ( md5_update.uuid == md5.uuid );
    REQUIRE ( md5_update.index_num == 18 );
    REQUIRE ( md5_update.md5_sum == "17e8f0b4718aa78060a067fcee68513c" );
    REQUIRE ( md5_update.date == 101 );
    REQUIRE ( md5_update.sec_id == "new-md5-sec-id" );

    REQUIRE ( blacklibrary_db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    md5.index_num = 1000;
    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5) == 0 );
    BlackLibraryCommon::Md5Sum md5_update_index_num = blacklibrary_db.ReadMd5SumBySeqNum(md5.uuid, md5.seq_num);

    REQUIRE ( md5_update_index_num.md5_sum == md5.md5_sum );
    REQUIRE ( md5_update_index_num.date == md5.date );
    REQUIRE ( md5_update_index_num.version_num == md5.version_num );
    REQUIRE ( md5_update_index_num.sec_id == md5.sec_id );
    REQUIRE ( md5_update_index_num.seq_num == md5.seq_num );
    REQUIRE ( md5_update_index_num.index_num == md5.index_num );

    REQUIRE ( blacklibrary_db.DeleteMd5Sum(md5.uuid, md5.index_num) == 0 );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistByIndexNum(md5.uuid, md5.index_num) == false );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistBySecId(md5.uuid, md5.sec_id) == false );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistBySeqNum(md5.uuid, md5.seq_num) == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test reading md5s back ordered by index_num sqlite (pass)", "[single-file]" )
{
    njson config = GenerateDBTestConfig();
    BlackLibraryDB blacklibrary_db(config);

    BlackLibraryCommon::Md5Sum md5_0 = GenerateTestMd5Sum();

    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5_0) == 0 );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistByIndexNum(md5_0.uuid, md5_0.index_num) == true );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistBySecId(md5_0.uuid, md5_0.sec_id) == true );

    BlackLibraryCommon::Md5Sum md5_1 = GenerateTestMd5Sum();
    BlackLibraryCommon::Md5Sum md5_2 = GenerateTestMd5Sum();
    md5_1.md5_sum = DefaultTestMd5_1;
    md5_2.md5_sum = DefaultTestMd5_2;
    md5_1.index_num = 20;
    md5_2.index_num = 10;
    md5_1.sec_id = "md5-sec-id-1";
    md5_2.sec_id = "md5-sec-id-2";

    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5_1) == 0 );
    REQUIRE ( blacklibrary_db.CreateMd5Sum(md5_2) == 0 );

    REQUIRE ( blacklibrary_db.DoesMd5SumExistByIndexNum(md5_1.uuid, md5_1.index_num) == true );
    REQUIRE ( blacklibrary_db.DoesMd5SumExistByIndexNum(md5_2.uuid, md5_2.index_num) == true );

    std::vector<BlackLibraryCommon::Md5Sum> md5_sums = blacklibrary_db.GetMd5SumsFromUUID(md5_0.uuid);

    size_t lowest = md5_2.index_num;
    for (const auto & md5 : md5_sums)
    {
        REQUIRE ( lowest <= md5.index_num );
        lowest = md5.index_num;
    }

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
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
    REQUIRE ( read_refresh.uuid == refresh.uuid );
    REQUIRE ( read_refresh.refresh_date == refresh.refresh_date );

    auto next_refresh = blacklibrary_db.GetRefreshFromMinDate();

    REQUIRE ( next_refresh.uuid == refresh.uuid );
    REQUIRE ( next_refresh.refresh_date == refresh.refresh_date );

    REQUIRE ( blacklibrary_db.DeleteRefresh(refresh.uuid) == 0 );

    REQUIRE ( blacklibrary_db.DoesRefreshExist(refresh.uuid) == false );
    REQUIRE ( blacklibrary_db.DoesMinRefreshExist() == false );

    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

TEST_CASE( "Test teardown tmp db" )
{
    BlackLibraryCommon::RemovePath(DefaultTestDBPath);
}

} // namespace db
} // namespace core
} // namespace black_library
