/**
 * DBConnectionInterface.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_DBCONNECTIONINTERFACE_H__
#define __BLACK_LIBRARY_CORE_DB_DBCONNECTIONINTERFACE_H__

#include <string>
#include <sstream>
#include <vector>

#include <BlackLibraryDBDataTypes.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

class DBConnectionInterface
{
public:
    virtual ~DBConnectionInterface() {}

    virtual std::vector<DBEntry> ListEntries() const = 0;
    virtual std::vector<BlackLibraryCommon::Md5Sum> ListChecksums() const = 0;
    virtual std::vector<DBErrorEntry> ListErrorEntries() const = 0;

    virtual int CreateEntry(const DBEntry &entry) const = 0;
    virtual DBEntry ReadEntry(const std::string &uuid) const = 0;
    virtual int UpdateEntry(const DBEntry &entry) const = 0;
    virtual int DeleteEntry(const std::string &uuid) const = 0;

    virtual int CreateMd5Sum(const BlackLibraryCommon::Md5Sum &md5) const = 0;
    virtual BlackLibraryCommon::Md5Sum ReadMd5SumByIndexNum(const std::string &uuid, size_t index_num) const = 0;
    virtual BlackLibraryCommon::Md5Sum ReadMd5SumBySecId(const std::string &uuid, const std::string &sec_id) const = 0;
    virtual BlackLibraryCommon::Md5Sum ReadMd5SumBySeqNum(const std::string &uuid, const size_t &seq_num) const = 0;
    virtual int UpdateMd5SumByIndexNum(const BlackLibraryCommon::Md5Sum &md5) const = 0;
    virtual int UpdateMd5SumBySeqNum(const BlackLibraryCommon::Md5Sum &md5) const = 0;
    virtual int DeleteMd5Sum(const std::string &uuid, size_t index_num) const = 0;

    virtual int CreateRefresh(const DBRefresh &refresh) const = 0;
    virtual DBRefresh ReadRefresh(const std::string &uuid) const = 0;
    virtual int DeleteRefresh(const std::string &uuid) const = 0;

    virtual int CreateErrorEntry(const DBErrorEntry &entry) const = 0;
    virtual int DeleteErrorEntry(const std::string &uuid, size_t progress_num) const = 0;

    virtual DBBoolResult DoesEntryUrlExist(const std::string &url) const = 0;
    virtual DBBoolResult DoesEntryUUIDExist(const std::string &uuid) const = 0;
    virtual DBBoolResult DoesMd5SumExistByIndexNum(const std::string &uuid, size_t index_num) const = 0;
    virtual DBBoolResult DoesMd5SumExistBySecId(const std::string &uuid, const std::string &sec_id) const = 0;
    virtual DBBoolResult DoesMd5SumExistBySeqNum(const std::string &uuid, const size_t &seq_num) const = 0;
    virtual DBBoolResult DoesRefreshExist(const std::string &uuid) const = 0;
    virtual DBBoolResult DoesMinRefreshExist() const = 0;
    virtual DBBoolResult DoesErrorEntryExist(const std::string &uuid, size_t progress_num) const = 0;

    virtual DBStringResult GetDBVersion() const = 0;
    virtual DBStringResult GetEntryUUIDFromUrl(const std::string &url) const = 0;
    virtual DBStringResult GetEntryUrlFromUUID(const std::string &uuid) const = 0;

    virtual BlackLibraryCommon::Md5Sum GetMd5SumFromMd5Sum(const std::string &md5_sum, const std::string &uuid) const = 0;
    virtual std::vector<BlackLibraryCommon::Md5Sum> GetMd5SumsFromUUID(const std::string &uuid) const = 0;

    virtual uint16_t GetVersionFromMd5(const std::string &uuid, size_t index_num) const = 0;

    virtual DBRefresh GetRefreshFromMinDate() const = 0;

    virtual bool IsReady() const = 0;

private:

};

} // namespace db
} // namespace core
} // namespace black_library

#endif
