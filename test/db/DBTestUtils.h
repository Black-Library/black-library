/**
 * DBTestUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_DB_TEST_UTILS_H__
#define __BLACK_LIBRARY_CORE_DB_DB_TEST_UTILS_H__

#include <ConfigOperations.h>
#include <VersionOperations.h>

#include <BlackLibraryDBDataTypes.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

static constexpr const char DefaultTestDBPath[] = "/tmp/catalog.db";
static constexpr const char DefaultTestUUID[] = "55ee59ad-2feb-4196-960b-3226c65c80d5";
static constexpr const char DefaultTestMd5_0[] = "e3369f7882e827dfaabb8e0aad854f26";
static constexpr const char DefaultTestMd5_1[] = "5732cace2d9e3f1a1ad774b35ff2948b";
static constexpr const char DefaultTestMd5_2[] = "7262cace2d9e3f1a1ad774b55ff2928a";

njson GenerateDBTestConfig()
{
    njson j;
    j["config"]["db_path"] = DefaultTestDBPath;
    j["config"]["logger_path"] = "/tmp/";
    j["config"]["db_debug_log"] = true;
    j["config"]["db_version"] = "1.0";
    return j;
}

DBEntry GenerateTestWorkEntry()
{
    DBEntry work_entry;

    work_entry.uuid = DefaultTestUUID;
    work_entry.title = "black-title";
    work_entry.author = "black-author";
    work_entry.nickname = "black-nickname";
    work_entry.source = "black-source";
    work_entry.url = "black-url";
    work_entry.last_url = "black-last-url";
    work_entry.series = "black-series";
    work_entry.series_length = 10;
    work_entry.version = 20;
    work_entry.media_path = "black-media-path";
    work_entry.birth_date = 30;
    work_entry.check_date = 40;
    work_entry.update_date = 50;
    work_entry.user_contributed = 4004;

    return work_entry;
}

BlackLibraryCommon::Md5Sum GenerateTestMd5Sum()
{
    BlackLibraryCommon::Md5Sum md5;

    md5.uuid = DefaultTestUUID;
    md5.md5_sum = DefaultTestMd5_0;
    md5.index_num = 18;
    md5.date = 0;
    md5.sec_id = "section-identifier";
    md5.seq_num = 3;
    md5.version_num = 4;

    return md5;
}

DBRefresh GenerateTestRefresh()
{
    DBRefresh refresh;

    refresh.uuid = DefaultTestUUID;
    refresh.refresh_date = 946598400;

    return refresh;
}

DBErrorEntry GenerateTestErrorEntry()
{
    DBErrorEntry error_entry;

    error_entry.uuid = DefaultTestUUID;
    error_entry.progress_num = 12;

    return error_entry;
}

} // namespace db
} // namespace core
} // namespace black_library

#endif
