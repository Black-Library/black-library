#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

#include <ParserManager.h>

#include "../CommonTestUtils.h"

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryParsers = black_library::core::parsers;

TEST_CASE( "Generic parser manager tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    njson config = GenerateParserTestConfig();
    ParserManager ParserManager(config, nullptr);
    ParserManager.Stop();
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

TEST_CASE( "Parser manager current jobs tests (pass)", "[single-file]" )
{
    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
    BlackLibraryCommon::MakeDirectories(DefaultTestStoragePath);
    njson config = GenerateParserTestConfig();
    ParserManager ParserManager(config, nullptr);

    auto job_list = ParserManager.GetCurrentJobList();
    REQUIRE( job_list.size() == 0 );
    ParserManager.AddJob("some-uuid-0", RR_DUMMY_URL, RR_DUMMY_URL);
    job_list = ParserManager.GetCurrentJobList();
    REQUIRE( job_list.size() == 1 );
    REQUIRE( job_list[0].uuid == "some-uuid-0" );
    REQUIRE( job_list[0].job_status == BlackLibraryParsers::job_status_t::JOB_MANAGER_QUEUED );
    REQUIRE( job_list[0].is_error_job == false );
    ParserManager.RunOnce();
    job_list = ParserManager.GetCurrentJobList();
    REQUIRE( job_list.size() == 1 );
    REQUIRE( job_list[0].uuid == "some-uuid-0" );
    REQUIRE( job_list[0].job_status == BlackLibraryParsers::job_status_t::JOB_WORKER_QUEUED );
    REQUIRE( job_list[0].is_error_job == false );

    // allow worker to identify parse error
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
    ParserManager.RunOnce();
    std::this_thread::sleep_until(deadline);

    job_list = ParserManager.GetCurrentJobList();
    REQUIRE( job_list.size() == 1 );
    REQUIRE( job_list[0].uuid == "some-uuid-0" );
    REQUIRE( job_list[0].job_status == BlackLibraryParsers::job_status_t::JOB_ERROR );
    REQUIRE( job_list[0].is_error_job == false );
    ParserManager.Stop();

    BlackLibraryCommon::RemovePath(DefaultTestStoragePath);
}

} // namespace parsers
} // namespace core
} // namespace black_library
