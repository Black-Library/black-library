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
    std::vector<DBEntry> GetWorkEntryList();
    std::vector<DBMd5Sum> GetChecksumList();
    std::vector<DBErrorEntry> GetErrorEntryList();

    // back-end
    int CreateWorkEntry(const DBEntry &entry);
    DBEntry ReadWorkEntry(const std::string &uuid);
    int UpdateWorkEntry(const DBEntry &entry);
    int DeleteWorkEntry(const std::string &uuid);

    int CreateMd5Sum(const DBMd5Sum &md5);
    DBMd5Sum ReadMd5Sum(const std::string &uuid, size_t index_num);
    int UpdateMd5Sum(const DBMd5Sum &md5);
    int DeleteMd5Sum(const std::string &uuid, size_t index_num);

    int CreateRefresh(const DBRefresh &refresh);
    DBRefresh ReadRefresh(const std::string &uuid);
    int DeleteRefresh(const std::string &uuid);

    int CreateErrorEntry(const DBErrorEntry &entry);
    int DeleteErrorEntry(const std::string &uuid, size_t progress_num);

    bool DoesWorkEntryUrlExist(const std::string &url);
    bool DoesMd5SumExist(const std::string &uuid, size_t index_num);
    bool DoesRefreshExist(const std::string &uuid);
    bool DoesMinRefreshExist();
    bool DoesErrorEntryExist(const std::string &uuid, size_t progress_num);

    bool DoesWorkEntryUUIDExist(const std::string &uuid);

    DBStringResult GetDBVersion();
    DBStringResult GetWorkEntryUUIDFromUrl(const std::string &url);
    DBStringResult GetWorkEntryUrlFromUUID(const std::string &uuid);

    uint16_t GetVersionFromMd5(const std::string &uuid, size_t index_num);

    DBRefresh GetRefreshFromMinDate();

    bool IsReady();

private:
    std::string GetUUID();

    std::unique_ptr<DBConnectionInterface> database_connection_interface_;
    std::string logger_name_;
    std::mutex mutex_;
};

} // namespace db
} // namespace core
} // namespace black_library

#endif
