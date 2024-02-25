/**
 * BlackLibraryDBRESTAPI.cc
 */

#include <BlackLibraryRESTAPI.h>

namespace black_library {

namespace core {

namespace rest_api {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

BlackLibraryDBRESTAPI::BlackLibraryDBRESTAPI(const njson &config, const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db) :
    blacklibrary_db_(blacklibrary_db),
    endpoint_(nullptr),
    rest_router_(),
    address_(),
    logger_name_("rest_api"),
    port_number_(),
    num_threads_(),
    initialized_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    address_ = Pistache::Address("localhost", 8080);

    initialized_ = true;
}

} // namespace rest_api
} // namespace core
} // namespace black_library