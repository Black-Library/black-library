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
    j["config"]["db_debug_log"] = true;
    j["config"]["db_version"] = "1.0";
    return j;
}

DBEntry GenerateTestBlackEntry()
{
    DBEntry work_entry;

    work_entry.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
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

DBMd5Sum GenerateTestMd5Sum()
{
    DBMd5Sum md5;

    md5.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
    md5.index_num = 18;
    md5.md5_sum = "c1b30f495b8d0def09e0f6a25728cbfc";
    md5.date = 100;
    md5.url = "md5-url";
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
