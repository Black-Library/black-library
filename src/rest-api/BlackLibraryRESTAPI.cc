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
    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    bool logger_level = BlackLibraryCommon::DefaultLogLevel;
    if (nconfig.contains("api_debug_log"))
    {
        logger_level = nconfig["api_debug_log"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI");

    address_ = Pistache::Address("localhost", port_number_);

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI address");

    endpoint_ = std::make_shared<Pistache::Http::Endpoint>(address_);

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI endpoint");

    endpoint_->init((Pistache::Http::Endpoint::options().threads(num_threads_)));

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI routes");

    SetRoutes();

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI endpoint handler");

    endpoint_->setHandler(rest_router_.handler());

    initialized_ = true;
    done_ = false;

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI endpoint thread");

    endpoint_thread_ = std::thread([this](){
        try
        {
            BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibraryAPI endpoint serve");
            endpoint_->serve();
        }
        catch (const std::runtime_error &ex)
        {
            BlackLibraryCommon::LogError(logger_name_, "BlackLibraryAPI runtime error {}", ex.what());
        }
        catch(const std::exception& ex)
        {
            BlackLibraryCommon::LogError(logger_name_, "BlackLibraryAPI exception {}", ex.what());
        }
    });
}

int BlackLibraryDBRESTAPI::SetRoutes()
{
    Pistache::Rest::Routes::Get(rest_router_, "/v1/ready", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::IsReadyAPI, this));

    Pistache::Rest::Routes::Get(rest_router_, "/v1/work_entry/all", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ListEntriesAPI, this));
    Pistache::Rest::Routes::Get(rest_router_, "/v1/check_sum/all", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ListChecksumsAPI, this));
    Pistache::Rest::Routes::Get(rest_router_, "/v1/error_entry/all", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ListErrorEntriesAPI, this));

    Pistache::Rest::Routes::Get(rest_router_, "/v1/work_entry", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ReadWorkEntryAPI, this));
    Pistache::Rest::Routes::Get(rest_router_, "/v1/check_sum", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ReadMd5SumAPI, this));

    Pistache::Rest::Routes::Get(rest_router_, "/v1/refresh/:uuid", Pistache::Rest::Routes::bind(&BlackLibraryDBRESTAPI::ReadRefresh, this));

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
    njson entry_list {};
    try
    {
        const std::string json = request.body();
        std::vector<BlackLibraryDB::DBEntry> entries = blacklibrary_db_->GetWorkEntryList();
        for (const auto& entry : entries)
        {
            entry_list.emplace_back(entry);
        }
        
        response.send(Pistache::Http::Code::Ok, entry_list.dump(4), MIME(Text, Plain));
    }
    catch (const std::runtime_error &ex)
    {
        response.send(Pistache::Http::Code::Not_Found, ex.what(), MIME(Text, Plain));
    }
    catch (...)
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed list entries API {}", entry_list.size());
        response.send(Pistache::Http::Code::Internal_Server_Error, "Internal error", MIME(Text, Plain));
    }
}

void BlackLibraryDBRESTAPI::ListChecksumsAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    njson checksum_list {};
    try
    {
        const std::string json = request.body();
        std::vector<BlackLibraryCommon::Md5Sum> checksums = blacklibrary_db_->GetChecksumList();
        for (const auto& checksum : checksums)
        {
            checksum_list.emplace_back(checksum);
        }
        
        response.send(Pistache::Http::Code::Ok, checksum_list.dump(4), MIME(Text, Plain));
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

void BlackLibraryDBRESTAPI::ListErrorEntriesAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    njson error_entry_list {};
    try
    {
        const std::string json = request.body();
        std::vector<BlackLibraryDB::DBErrorEntry> error_entries = blacklibrary_db_->GetErrorEntryList();
        for (const auto& error_entry : error_entries)
        {
            error_entry_list.emplace_back(error_entry);
        }
        
        response.send(Pistache::Http::Code::Ok, error_entry_list.dump(4), MIME(Text, Plain));
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

void BlackLibraryDBRESTAPI::CreateWorkEntryAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        njson entry_json = { json };
        BlackLibraryDB::DBEntry entry = entry_json.template get<BlackLibraryDB::DBEntry>();
        int res = blacklibrary_db_->CreateWorkEntry(entry);
        njson res_json = { res };
        response.send(Pistache::Http::Code::Ok, res_json.dump(4), MIME(Text, Plain));
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
        const njson n_json = njson::parse(json);
        if (!n_json.contains("uuid"))
        {
            throw std::runtime_error("no uuid");
        }
        const std::string uuid = n_json["uuid"];
        BlackLibraryDB::DBEntry entry = blacklibrary_db_->ReadWorkEntry(uuid);
        njson entry_json = entry;
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

void BlackLibraryDBRESTAPI::ReadMd5SumAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        const njson n_json = njson::parse(json);
        if (!n_json.contains("uuid"))
        {
            throw std::runtime_error("no uuid");
        }
        const std::string uuid = n_json["uuid"];
        if (n_json.contains("index_num"))
        {
            size_t index_num = n_json["index_num"];
            BlackLibraryCommon::Md5Sum md5_sum = blacklibrary_db_->ReadMd5SumByIndexNum(uuid, index_num);
            njson md5_sum_json = md5_sum;
            response.send(Pistache::Http::Code::Ok, md5_sum_json.dump(4), MIME(Text, Plain));
        }
        else if (n_json.contains("sec_id"))
        {
            std::string sec_id = n_json["sec_id"];
            BlackLibraryCommon::Md5Sum md5_sum = blacklibrary_db_->ReadMd5SumBySecId(uuid, sec_id);
            njson md5_sum_json = md5_sum;
            response.send(Pistache::Http::Code::Ok, md5_sum_json.dump(4), MIME(Text, Plain));
        }
        else if (n_json.contains("seq_num"))
        {
            size_t seq_num = n_json["seq_num"];
            BlackLibraryCommon::Md5Sum md5_sum = blacklibrary_db_->ReadMd5SumBySeqNum(uuid, seq_num);
            njson md5_sum_json = md5_sum;
            response.send(Pistache::Http::Code::Ok, md5_sum_json.dump(4), MIME(Text, Plain));
        }
        else
        {
            BlackLibraryCommon::LogError(logger_name_, "ReadMd5SumAPI failed, missing key");
        }
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

void BlackLibraryDBRESTAPI::ReadRefresh(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        const njson n_json = njson::parse(json);
        if (!n_json.contains("uuid"))
        {
            throw std::runtime_error("no uuid");
        }
        const std::string uuid = n_json["uuid"];
        BlackLibraryDB::DBRefresh refresh = blacklibrary_db_->ReadRefresh(uuid);
        njson refresh_json = refresh;
        response.send(Pistache::Http::Code::Ok, refresh_json.dump(4), MIME(Text, Plain));
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

bool BlackLibraryDBRESTAPI::IsReady()
{
    return initialized_;
}

void BlackLibraryDBRESTAPI::IsReadyAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response)
{
    try
    {
        const std::string json = request.body();
        njson ready_json { initialized_ };
        response.send(Pistache::Http::Code::Ok, ready_json.dump(4), MIME(Text, Plain));
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
