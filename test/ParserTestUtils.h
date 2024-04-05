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

namespace BlackLibraryDb = black_library::core::db;

BlackLibraryDb::DBEntry GenerateTestWorkEntry()
{
    BlackLibraryDb::DBEntry work_entry;

    work_entry.uuid = RR_DUMMY_UUID;
    work_entry.title = "black-title";
    work_entry.author = "black-author";
    work_entry.nickname = "black-nickname";
    work_entry.source = "black-source";
    work_entry.url = RR_DUMMY_URL;
    work_entry.last_url = RR_URL_5;
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
    md5_0.sec_id = RR_SEC_ID_0;
    md5_0.seq_num = RR_SEQ_NUM_0;

    md5_1.uuid = RR_DUMMY_UUID;
    md5_1.index_num = 1;
    md5_1.md5_sum = RR_MD5_1;
    md5_1.sec_id = RR_SEC_ID_1;
    md5_1.seq_num = RR_SEQ_NUM_1;

    md5_2.uuid = RR_DUMMY_UUID;
    md5_2.index_num = 2;
    md5_2.md5_sum = RR_MD5_2;
    md5_2.sec_id = RR_SEC_ID_2;
    md5_2.seq_num = RR_SEQ_NUM_2;

    md5_3.uuid = RR_DUMMY_UUID;
    md5_3.index_num = 3;
    md5_3.md5_sum = RR_MD5_3;
    md5_3.sec_id = RR_SEC_ID_3;
    md5_3.seq_num = RR_SEQ_NUM_3;

    md5_4.uuid = RR_DUMMY_UUID;
    md5_4.index_num = 4;
    md5_4.md5_sum = RR_MD5_4;
    md5_4.sec_id = RR_SEC_ID_4;
    md5_4.seq_num = RR_SEQ_NUM_4;

    md5_5.uuid = RR_DUMMY_UUID;
    md5_5.index_num = 5;
    md5_5.md5_sum = RR_MD5_5;
    md5_5.sec_id = RR_SEC_ID_5;
    md5_5.seq_num = RR_SEQ_NUM_5;

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