/**
 * CommonTestUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_COMMON_TEST_UTILS_H__
#define __BLACK_LIBRARY_CORE_PARSERS_COMMON_TEST_UTILS_H__

#include <ConfigOperations.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

static constexpr const char DefaultTestDbPath[] = "/tmp/catalog.db";
static constexpr const char DefaultTestLogPath[] = "/tmp/log";
static constexpr const char DefaultTestStoragePath[] = "/tmp/store";

static constexpr const char AO3_DUMMY_URL[] = "https://archiveofourown.org/works/0000000";
static constexpr const char RR_DUMMY_URL[] = "https://www.royalroad.com/fiction/00000/some-fiction";
static constexpr const char SBF_DUMMY_URL[] = "https://forums.spacebattles.com/threads/some-fiction-name.0000000/";
static constexpr const char SVF_DUMMY_URL[] = "https://forums.sufficientvelocity.com/threads/some-fiction-name.000000/";
static constexpr const char WP_DUMMY_URL[] = "https://some-name.wordpress.com/";

static constexpr const char RR_DUMMY_UUID[] = "b0ad76bc-39d9-40ac-8de7-c5020568526d";

static constexpr const char RR_DUMMY_CONTENT_0[] = "dummy content 0 to hash";
static constexpr const char RR_DUMMY_CONTENT_1[] = "dummy content 1 to hash";
static constexpr const char RR_DUMMY_CONTENT_2[] = "dummy content 2 to hash";
static constexpr const char RR_DUMMY_CONTENT_3[] = "dummy content 3 to hash";
static constexpr const char RR_DUMMY_CONTENT_4[] = "dummy content 4 to hash";
static constexpr const char RR_DUMMY_CONTENT_5[] = "dummy content 5 to hash";

static constexpr const char RR_MD5_0[] = "25ae80c3e257f51febe1df184ab45a6c";
static constexpr const char RR_MD5_1[] = "cc73b8f51042ef6c7f9eab18d01fb9cf";
static constexpr const char RR_MD5_2[] = "b3663b1b3f39ed63db19e8697e80c06a";
static constexpr const char RR_MD5_3[] = "2ec57486c6a83b95695815fb90760ee3";
static constexpr const char RR_MD5_4[] = "2182958d369511a4e3e703f47facfd25";
static constexpr const char RR_MD5_5[] = "0c67eaf291a40b9d6e2f1b1d1c8d4d4e";

static constexpr const char RR_URL_0[] = "https://www.royalroad.com/fiction/00000/some-fiction/0";
static constexpr const char RR_URL_1[] = "https://www.royalroad.com/fiction/00000/some-fiction/1";
static constexpr const char RR_URL_2[] = "https://www.royalroad.com/fiction/00000/some-fiction/2";
static constexpr const char RR_URL_3[] = "https://www.royalroad.com/fiction/00000/some-fiction/3";
static constexpr const char RR_URL_4[] = "https://www.royalroad.com/fiction/00000/some-fiction/4";
static constexpr const char RR_URL_5[] = "https://www.royalroad.com/fiction/00000/some-fiction/5";

njson GenerateParserTestConfig()
{
    njson j;

    j["config"]["db_path"] = DefaultTestDbPath;
    j["config"]["logger_path"] = DefaultTestLogPath;
    j["config"]["storage_path"] = DefaultTestStoragePath;
    j["config"]["worker_debug_log"] = true;
    j["config"]["parser_debug_log"] = true;

    return j;
}

} // namespace parsers
} // namespace core
} // namespace black_library

#endif