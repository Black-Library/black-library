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
    ~BlackLibraryDBRESTAPI();

    // std::vector<DBEntry> ListEntries() const;
    // std::vector<BlackLibraryCommon::Md5Sum> ListChecksums() const;
    // std::vector<DBErrorEntry> ListErrorEntries() const;

    // int CreateDBVersion(const std::string &db_version) const;
    // int CreateUser(const DBUser &user) const;
    // int CreateMediaType(const std::string &media_type_name) const;
    // int CreateMediaSubtype(const std::string &media_subtype_name, const std::string &media_type_name) const;
    // int CreateSource(const DBSource &source) const;

    // int CreateEntry(const DBEntry &entry) const override;
    // DBEntry ReadEntry(const std::string &uuid) const override;
    // int UpdateEntry(const DBEntry &entry) const override;
    // int DeleteEntry(const std::string &uuid) const override;

    // int CreateMd5Sum(const BlackLibraryCommon::Md5Sum &md5) const override;
    // BlackLibraryCommon::Md5Sum ReadMd5SumByIndexNum(const std::string &uuid, size_t index_num) const override;
    // BlackLibraryCommon::Md5Sum ReadMd5SumBySecId(const std::string &uuid, const std::string &sec_id) const override;
    // BlackLibraryCommon::Md5Sum ReadMd5SumBySeqNum(const std::string &uuid, const size_t &seq_num) const override;
    // int UpdateMd5SumByIndexNum(const BlackLibraryCommon::Md5Sum &md5) const override;
    // int UpdateMd5SumBySeqNum(const BlackLibraryCommon::Md5Sum &md5) const override;
    // int DeleteMd5Sum(const std::string &uuid, size_t index_num) const override;

    // int CreateRefresh(const DBRefresh &refresh) const override;
    // DBRefresh ReadRefresh(const std::string &uuid) const override;
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

    // bool IsReady() const;

    int SetRoutes();

private:
    std::shared_ptr<BlackLibraryDB::BlackLibraryDB> blacklibrary_db_;
    std::shared_ptr<Pistache::Http::Endpoint> endpoint_;
    Pistache::Rest::Router rest_router_;
    Pistache::Address address_;
    std::string logger_name_;
    uint16_t port_number_;
    uint16_t num_threads_;
    bool initialized_;
};

} // namespace rest_api
} // namespace core
} // namespace black_library

#endif
