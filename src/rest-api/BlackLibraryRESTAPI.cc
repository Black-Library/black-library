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
    endpoint_thread_(),
    logger_name_("rest_api"),
    port_number_(8080),
    num_threads_(4),
    done_(true),
    initialized_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    address_ = Pistache::Address("localhost", port_number_);

    endpoint_ = std::make_shared<Pistache::Http::Endpoint>(address_);

    endpoint_->init((Pistache::Http::Endpoint::options().threads(num_threads_)));

    SetRoutes();

    endpoint_->setHandler(rest_router_.handler());

    initialized_ = true;
    done_ = false;

    endpoint_thread_ = std::thread([this](){
            endpoint_->serve();
    });
}

int BlackLibraryDBRESTAPI::SetRoutes()
{
    // Pistache::Rest::Routes::Post(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));
    // Pistache::Rest::Routes::Get(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));
    // Pistache::Rest::Routes::Put(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));
    // Pistache::Rest::Routes::Delete(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));
    // Pistache::Rest::Routes::Post(rest_router_, "/widget", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));

    return 0;
}

int BlackLibraryDBRESTAPI::Stop()
{
    done_ = true;
    endpoint_->shutdown();

    BlackLibraryCommon::LogWarn(logger_name_, "Joining rest api thread");

    if (endpoint_thread_.joinable())
        endpoint_thread_.join();

    return 0;
}

} // namespace rest_api
} // namespace core
} // namespace black_library
