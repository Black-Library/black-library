#include <queue>

#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <VersionOperations.h>

#include "../CommonTestUtils.h"

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

TEST_CASE( "Generic Md5Sum tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::Md5Sum md5_0;
    BlackLibraryCommon::Md5Sum md5_1;

    md5_0.seq_num = 0;
    md5_1.seq_num = 1;

    std::priority_queue<BlackLibraryCommon::Md5Sum, std::vector<BlackLibraryCommon::Md5Sum>, BlackLibraryCommon::Md5SumGreaterThanBySeqNum> md5_seq_num_queue;

    md5_seq_num_queue.push(md5_0);
    md5_seq_num_queue.push(md5_1);

    REQUIRE (md5_seq_num_queue.top().seq_num == 0 );
}

TEST_CASE( "Generic version GetMD5Hash tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    auto config = GenerateParserTestConfig();

    auto content_md5_0 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_0);
    REQUIRE ( content_md5_0 == RR_MD5_0 );

    auto content_md5_1 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_1);
    REQUIRE ( content_md5_1 == RR_MD5_1 );

    auto content_md5_2 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_2);
    REQUIRE ( content_md5_2 == RR_MD5_2 );

    auto content_md5_3 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_3);
    REQUIRE ( content_md5_3 == RR_MD5_3 );

    auto content_md5_4 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_4);
    REQUIRE ( content_md5_4 == RR_MD5_4 );

    auto content_md5_5 = BlackLibraryCommon::GetMD5Hash(RR_DUMMY_CONTENT_5);
    REQUIRE ( content_md5_5 == RR_MD5_5 );

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Generic version GetWorkChapterSecIdFromUrl tests (pass)", "[single-file]" )
{
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSecIdFromUrl(RR_DUMMY_CHAPTER_URL) == "chapter-name" );
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSecIdFromUrl(SBF_DUMMY_CHAPTER_URL_0) == "page-1" );
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSecIdFromUrl(SBF_DUMMY_CHAPTER_URL_1) == "page-2" );
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSecIdFromUrl(SVF_DUMMY_CHAPTER_URL) == "page-1" );
}

TEST_CASE( "Generic version GetWorkChapterSeqNumFromUrl tests (pass)", "[single-file]" )
{
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(RR_DUMMY_CHAPTER_URL) == 1234567 );
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(SBF_DUMMY_CHAPTER_URL_0) == 12345678 );
    REQUIRE ( BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(SVF_DUMMY_CHAPTER_URL) == 12345678 );
}

TEST_CASE( "Generic version GetWorkNumFromUrl tests (pass)", "[single-file]" )
{
    REQUIRE ( BlackLibraryCommon::GetWorkNumFromUrl(RR_DUMMY_CHAPTER_URL) == 12345 );
    REQUIRE ( BlackLibraryCommon::GetWorkNumFromUrl(SBF_DUMMY_CHAPTER_URL_0) == 1234567 );
    REQUIRE ( BlackLibraryCommon::GetWorkNumFromUrl(SVF_DUMMY_CHAPTER_URL) == 123456 );
}

TEST_CASE( "Clean up in case of failure" )
{
    BlackLibraryCommon::RemovePath(DefaultTestDbPath);
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
