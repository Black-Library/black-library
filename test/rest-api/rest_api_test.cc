#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>
#include <BlackLibraryRESTAPI.h>

#include <DBTestUtils.h>

namespace black_library {

namespace core {

namespace rest_api {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

TEST_CASE( "REST API Generic constructor destructor tests (pass)", "[single-file]")
{
    auto config = BlackLibraryDB::GenerateDBTestConfig();
    auto blacklibrary_db = std::make_shared<BlackLibraryDB::BlackLibraryDB>(config);

    REQUIRE ( blacklibrary_db->IsReady() == true );

    auto blacklibrary_api = std::make_shared<BlackLibraryDBRESTAPI>(config, blacklibrary_db);

    REQUIRE ( blacklibrary_api->IsReady() == true );

    blacklibrary_api->Stop();

    BlackLibraryCommon::RemovePath(BlackLibraryDB::DefaultTestDBPath);
}

} // namespace rest_api
} // namespace core
} // namespace black_library
