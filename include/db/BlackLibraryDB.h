/**
 * BlackLibraryDB.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_BLACKLIBRARYDB_H__
#define __BLACK_LIBRARY_CORE_DB_BLACKLIBRARYDB_H__

#include <memory>
#include <mutex>
#include <string>

#include <ConfigOperations.h>

#include <DBConnectionInterface.h>

namespace black_library {

namespace core {

namespace db {

class BlackLibraryDB {
public:
    explicit BlackLibraryDB(const njson &config);
    ~BlackLibraryDB();

    // front-end
    std::vector<DBEntry> GetStagingEntryList();
    std::vector<DBEntry> GetBlackEntryList();
    std::vector<DBMd5Sum> GetChecksumList();
    std::vector<DBErrorEntry> GetErrorEntryList();

    // back-end
    int CreateStagingEntry(const DBEntry &entry);
    DBEntry ReadStagingEntry(const std::string &uuid);
    int UpdateStagingEntry(const DBEntry &entry);
    int DeleteStagingEntry(const std::string &uuid);

    int CreateBlackEntry(const DBEntry &entry);
    DBEntry ReadBlackEntry(const std::string &uuid);
    int UpdateBlackEntry(const DBEntry &entry);
    int DeleteBlackEntry(const std::string &uuid);

    int CreateMd5Sum(const DBMd5Sum &md5);
    DBMd5Sum ReadMd5Sum(const std::string &uuid, size_t index_num);
    int UpdateMd5Sum(const DBMd5Sum &md5);
    int DeleteMd5Sum(const std::string &uuid, size_t index_num);

    int CreateRefresh(const DBRefresh &refresh);
    DBRefresh ReadRefresh(const std::string &uuid);
    int DeleteRefresh(const std::string &uuid);

    int CreateErrorEntry(const DBErrorEntry &entry);
    int DeleteErrorEntry(const std::string &uuid, size_t progress_num);

    bool DoesStagingEntryUrlExist(const std::string &url);
    bool DoesBlackEntryUrlExist(const std::string &url);
    bool DoesMd5SumExist(const std::string &uuid, size_t index_num);
    bool DoesRefreshExist(const std::string &uuid);
    bool DoesMinRefreshExist();
    bool DoesErrorEntryExist(const std::string &uuid, size_t progress_num);

    bool DoesStagingEntryUUIDExist(const std::string &uuid);
    bool DoesBlackEntryUUIDExist(const std::string &uuid);

    DBStringResult GetStagingEntryUUIDFromUrl(const std::string &url);
    DBStringResult GetStagingEntryUrlFromUUID(const std::string &uuid);

    DBStringResult GetBlackEntryUUIDFromUrl(const std::string &url);
    DBStringResult GetBlackEntryUrlFromUUID(const std::string &uuid);

    uint16_t GetVersionFromMd5(const std::string &uuid, size_t index_num);

    DBRefresh GetRefreshFromMinDate();

    bool IsReady();

private:
    std::string GetUUID();

    std::unique_ptr<DBConnectionInterface> database_connection_interface_;
    std::mutex mutex_;
};

} // namespace db
} // namespace core
} // namespace black_library

#endif
