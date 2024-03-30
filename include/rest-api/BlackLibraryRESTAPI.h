/**
 * BlackLibraryRESTAPI.h
 */

#ifndef __BLACK_LIBRARY_CORE_REST_API_BLACKLIBRARYRESTAPI_H__
#define __BLACK_LIBRARY_CORE_REST_API_BLACKLIBRARYRESTAPI_H__

#include <memory>
#include <vector>

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <pistache/router.h>

#include <ConfigOperations.h>

// #include <DBConnectionInterface.h>
#include <BlackLibraryDB.h>

namespace black_library {

namespace core {

namespace rest_api {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

class BlackLibraryDBRESTAPI 
// : public DBConnectionInterface
{
public:
    explicit BlackLibraryDBRESTAPI(const njson &config, const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db);

    void ListEntriesAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void ListChecksumsAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void ListErrorEntriesAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

    void CreateWorkEntryAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void ReadWorkEntryAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);


    // int CreateMd5Sum(const BlackLibraryCommon::Md5Sum &md5) const override;
    void ReadMd5SumByIndexNumAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void ReadMd5SumBySecIdAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    void ReadMd5SumBySeqNumAPI(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    // int UpdateMd5SumByIndexNum(const BlackLibraryCommon::Md5Sum &md5) const override;
    // int UpdateMd5SumBySeqNum(const BlackLibraryCommon::Md5Sum &md5) const override;
    // int DeleteMd5Sum(const std::string &uuid, size_t index_num) const override;

    // int CreateRefresh(const DBRefresh &refresh) const override;
    void ReadRefresh(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);
    // int DeleteRefresh(const std::string &uuid) const override;

    // int CreateErrorEntry(const DBErrorEntry &entry) const override;
    // int DeleteErrorEntry(const std::string &uuid, size_t progress_num) const override;

    // DBBoolResult DoesEntryUrlExist(const std::string &url) const override;
    // DBBoolResult DoesEntryUUIDExist(const std::string &uuid) const override;
    // DBBoolResult DoesMd5SumExistByIndexNum(const std::string &uuid, size_t index_num) const override;
    // DBBoolResult DoesMd5SumExistBySecId(const std::string &uuid, const std::string &sec_id) const override;
    // DBBoolResult DoesMd5SumExistBySeqNum(const std::string &uuid, const size_t &seq_num) const override;
    // DBBoolResult DoesRefreshExist(const std::string &uuid) const override;
    // DBBoolResult DoesMinRefreshExist() const override;
    // DBBoolResult DoesErrorEntryExist(const std::string &uuid, size_t progress_num) const override;

    // DBStringResult GetDBVersion() const override;
    // DBStringResult GetEntryUUIDFromUrl(const std::string &url) const override;
    // DBStringResult GetEntryUrlFromUUID(const std::string &uuid) const override;

    // BlackLibraryCommon::Md5Sum GetMd5SumFromMd5Sum(const std::string &md5_sum, const std::string &uuid) const override;
    // std::vector<BlackLibraryCommon::Md5Sum> GetMd5SumsFromUUID(const std::string &uuid) const override;

    // uint16_t GetVersionFromMd5(const std::string &uuid, size_t index_num) const override;

    // DBRefresh GetRefreshFromMinDate() const override;

    void IsReady(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

    int SetRoutes();

    int Stop();

private:
    std::shared_ptr<BlackLibraryDB::BlackLibraryDB> blacklibrary_db_;
    std::shared_ptr<Pistache::Http::Endpoint> endpoint_;
    Pistache::Rest::Router rest_router_;
    Pistache::Address address_;
    std::thread endpoint_thread_;
    std::string logger_name_;
    uint16_t port_number_;
    uint16_t num_threads_;
    bool done_;
    bool initialized_;
};

} // namespace rest_api
} // namespace core
} // namespace black_library

#endif
