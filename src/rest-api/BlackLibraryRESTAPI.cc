/**
 * BlackLibraryDBRESTAPI.cc
 */

#include <LogOperations.h>

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

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    bool logger_level = BlackLibraryCommon::DefaultLogLevel;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI");

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
    Pistache::Rest::Routes::Get(rest_router_, "/work_entry/all", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ListEntriesAPI, this));

    // Pistache::Rest::Routes::Post(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDB::BlackLibraryDB::CreateWorkEntry, blacklibrary_db_));
    Pistache::Rest::Routes::Get(rest_router_, "/work_entry/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ReadWorkEntryAPI, this));
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

void BlackLibraryDBRESTAPI::ListEntriesAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        std::vector<BlackLibraryDB::DBEntry> entries = blacklibrary_db_->GetWorkEntryList();
        njson entry_list {};
        for (const auto& entry : entries)
        {
            entry_list.insert(entry_list.end(), entry);
        }
        
        response.send(Pistache::Http::Code::Ok, entry_list.dump(4), MIME(Text, Plain));
    }
    catch (const std::runtime_error &ex)
    {
        response.send(Pistache::Http::Code::Not_Found, ex.what(), MIME(Text, Plain));
    }
    catch (...)
    {
        response.send(Pistache::Http::Code::Internal_Server_Error, "Internal error", MIME(Text, Plain));
    }
}

void BlackLibraryDBRESTAPI::ReadWorkEntryAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        const std::string uuid = request.param(":uuid").as<std::string>();
        BlackLibraryDB::DBEntry entry = blacklibrary_db_->ReadWorkEntry(uuid);
        njson entry_json { entry };
        response.send(Pistache::Http::Code::Ok, entry_json.dump(4), MIME(Text, Plain));
    }
    catch (const std::runtime_error &ex)
    {
        response.send(Pistache::Http::Code::Not_Found, ex.what(), MIME(Text, Plain));
    }
    catch (...)
    {
        response.send(Pistache::Http::Code::Internal_Server_Error, "Internal error", MIME(Text, Plain));
    }
}

} // namespace rest_api
} // namespace core
} // namespace black_library
