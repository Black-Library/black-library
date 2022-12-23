/**
 * DBTestUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_DB_TEST_UTILS_H__
#define __BLACK_LIBRARY_CORE_DB_DB_TEST_UTILS_H__

#include <ConfigOperations.h>

#include <BlackLibraryDBDataTypes.h>

namespace black_library {

namespace core {

namespace db {

static constexpr const char DefaultTestDBPath[] = "/tmp/catalog.db";

njson GenerateDBTestConfig()
{
    njson j;
    j["config"]["db_path"] = DefaultTestDBPath;
    j["config"]["logger_path"] = "/tmp/";
    j["config"]["db_debug_log"] = false;
    return j;
}

DBEntry GenerateTestBlackEntry()
{
    DBEntry black_entry;

    black_entry.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
    black_entry.title = "black-title";
    black_entry.author = "black-author";
    black_entry.nickname = "black-nickname";
    black_entry.source = "black-source";
    black_entry.url = "black-url";
    black_entry.last_url = "black-last-url";
    black_entry.series = "black-series";
    black_entry.series_length = 10;
    black_entry.version = 20;
    black_entry.media_path = "black-media-path";
    black_entry.birth_date = 30;
    black_entry.check_date = 40;
    black_entry.update_date = 50;
    black_entry.user_contributed = 4004;

    return black_entry;
}

DBEntry GenerateTestStagingEntry()
{
    DBEntry staging_entry;

    staging_entry.uuid = "75ee5fad-2deb-4436-120c-3226ceeeaed6";
    staging_entry.title = "staging-title";
    staging_entry.author = "staging-author";
    staging_entry.nickname = "staging-nickname";
    staging_entry.source = "staging-source";
    staging_entry.url = "staging-url";
    staging_entry.last_url = "staging-last-url";
    staging_entry.series = "staging-series";
    staging_entry.series_length = 15;
    staging_entry.version = 25;
    staging_entry.media_path = "staging-media-path";
    staging_entry.birth_date = 35;
    staging_entry.check_date = 45;
    staging_entry.update_date = 55;
    staging_entry.user_contributed = 4004;

    return staging_entry;
}

DBMd5Sum GenerateTestMd5Sum()
{
    DBMd5Sum md5;

    md5.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
    md5.index_num = 18;
    md5.md5_sum = "c1b30f495b8d0def09e0f6a25728cbfc";
    md5.version_num = 4;

    return md5;
}

DBRefresh GenerateTestRefresh()
{
    DBRefresh refresh;

    refresh.uuid = "50470924-7e39-46cb-997c-aa8d882e1c59";
    refresh.refresh_date = 946598400;

    return refresh;
}

DBErrorEntry GenerateTestErrorEntry()
{
    DBErrorEntry error_entry;

    error_entry.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
    error_entry.progress_num = 12;

    return error_entry;
}

} // namespace db
} // namespace core
} // namespace black_library

#endif
