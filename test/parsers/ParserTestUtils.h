/**
 * ParserTestUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_TEST_UTILS_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_TEST_UTILS_H__

#include <ConfigOperations.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

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

static constexpr const char RR_MD5_0[] = "c353b81d9f3bcc779c05dcba168fb4d6";
static constexpr const char RR_MD5_1[] = "ca24ea7ffe0e27b1f988bcb389660d9f";
static constexpr const char RR_MD5_2[] = "53c5559beb9b465897ba1f1e4064985b";
static constexpr const char RR_MD5_3[] = "20cf030687bea368437d453fd9f4a546";
static constexpr const char RR_MD5_4[] = "20f190598790f44930a0b15af67e15e5";
static constexpr const char RR_MD5_5[] = "762a218ffcca26773409886afcd25680";

static constexpr const char RR_URL_0[] = "https://www.royalroad.com/fiction/00000/some-fiction/0";
static constexpr const char RR_URL_1[] = "https://www.royalroad.com/fiction/00000/some-fiction/1";
static constexpr const char RR_URL_2[] = "https://www.royalroad.com/fiction/00000/some-fiction/2";
static constexpr const char RR_URL_3[] = "https://www.royalroad.com/fiction/00000/some-fiction/3";
static constexpr const char RR_URL_4[] = "https://www.royalroad.com/fiction/00000/some-fiction/4";
static constexpr const char RR_URL_5[] = "https://www.royalroad.com/fiction/00000/some-fiction/5";

njson GenerateParserTestConfig()
{
    njson j;

    j["config"]["logger_path"] = DefaultTestLogPath;
    j["config"]["storage_path"] = DefaultTestStoragePath;
    j["config"]["worker_debug_log"] = true;
    j["config"]["parser_debug_log"] = true;

    return j;
}

std::unordered_map<uint8_t, BlackLibraryCommon::Md5Sum> GenerateFullTestMd5Map()
{
    std::unordered_map<uint8_t, BlackLibraryCommon::Md5Sum> md5_map;

    BlackLibraryCommon::Md5Sum md5_0;
    BlackLibraryCommon::Md5Sum md5_1;
    BlackLibraryCommon::Md5Sum md5_2;
    BlackLibraryCommon::Md5Sum md5_3;
    BlackLibraryCommon::Md5Sum md5_4;
    BlackLibraryCommon::Md5Sum md5_5;

    md5_0.uuid = RR_DUMMY_UUID;
    md5_0.index_num = 0;
    md5_0.md5_sum = RR_MD5_0;
    md5_0.url = RR_URL_0;

    md5_1.uuid = RR_DUMMY_UUID;
    md5_1.index_num = 1;
    md5_1.md5_sum = RR_MD5_1;
    md5_1.url = RR_URL_1;

    md5_2.uuid = RR_DUMMY_UUID;
    md5_2.index_num = 2;
    md5_2.md5_sum = RR_MD5_2;
    md5_2.url = RR_URL_2;

    md5_3.uuid = RR_DUMMY_UUID;
    md5_3.index_num = 3;
    md5_3.md5_sum = RR_MD5_3;
    md5_3.url = RR_URL_3;

    md5_4.uuid = RR_DUMMY_UUID;
    md5_4.index_num = 4;
    md5_4.md5_sum = RR_MD5_4;
    md5_4.url = RR_URL_4;

    md5_5.uuid = RR_DUMMY_UUID;
    md5_5.index_num = 5;
    md5_5.md5_sum = RR_MD5_5;
    md5_5.url = RR_URL_5;

    md5_map.emplace(0, md5_0);
    md5_map.emplace(1, md5_1);
    md5_map.emplace(2, md5_2);
    md5_map.emplace(3, md5_3);
    md5_map.emplace(4, md5_4);
    md5_map.emplace(5, md5_5);

    return md5_map;
}

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
