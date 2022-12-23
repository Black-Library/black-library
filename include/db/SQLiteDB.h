/**
 * SQLiteDB.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_SQLITEDB_H__
#define __BLACK_LIBRARY_CORE_DB_SQLITEDB_H__

#include <vector>

#include <sqlite3.h>

#include <DBConnectionInterface.h>

namespace black_library {

namespace core {

namespace db {

class SQLiteDB : public DBConnectionInterface
{
public:
    explicit SQLiteDB(const std::string &database_url);
    ~SQLiteDB();

    std::vector<DBEntry> ListEntries(entry_table_rep_t entry_type) const;
    std::vector<DBMd5Sum> ListChecksums() const;
    std::vector<DBErrorEntry> ListErrorEntries() const;

    int CreateUser(const DBUser &user) const;
    int CreateMediaType(const std::string &media_type_name) const;
    int CreateMediaSubtype(const std::string &media_subtype_name, const std::string &media_type_name) const;
    int CreateSource(const DBSource &source) const;

    int CreateEntry(const DBEntry &entry, entry_table_rep_t entry_type) const override;
    DBEntry ReadEntry(const std::string &uuid, entry_table_rep_t entry_type) const override;
    int UpdateEntry(const DBEntry &entry, entry_table_rep_t entry_type) const override;
    int DeleteEntry(const std::string &uuid, entry_table_rep_t entry_type) const override;

    int CreateMd5Sum(const DBMd5Sum &md5) const override;
    DBMd5Sum ReadMd5Sum(const std::string &uuid, size_t index_num) const override;
    int UpdateMd5Sum(const DBMd5Sum &md5) const override;
    int DeleteMd5Sum(const std::string &uuid, size_t index_num) const override;

    int CreateRefresh(const DBRefresh &refresh) const override;
    DBRefresh ReadRefresh(const std::string &uuid) const override;
    int DeleteRefresh(const std::string &uuid) const override;

    int CreateErrorEntry(const DBErrorEntry &entry) const override;
    int DeleteErrorEntry(const std::string &uuid, size_t progress_num) const override;

    DBBoolResult DoesEntryUrlExist(const std::string &url, entry_table_rep_t entry_type) const override;
    DBBoolResult DoesEntryUUIDExist(const std::string &uuid, entry_table_rep_t entry_type) const override;
    DBBoolResult DoesMd5SumExist(const std::string &uuid, size_t index_num) const override;
    DBBoolResult DoesRefreshExist(const std::string &uuid) const override;
    DBBoolResult DoesMinRefreshExist() const override;
    DBBoolResult DoesErrorEntryExist(const std::string &uuid, size_t progress_num) const override;

    DBStringResult GetEntryUUIDFromUrl(const std::string &url, entry_table_rep_t entry_type) const override;
    DBStringResult GetEntryUrlFromUUID(const std::string &uuid, entry_table_rep_t entry_type) const override;

    uint16_t GetVersionFromMd5(const std::string &uuid, size_t index_num) const override;

    DBRefresh GetRefreshFromMinDate() const override;

    bool IsReady() const;

private:
    int GenerateTables();

    int SetupDefaultTypeTables();
    int SetupDefaultEntryTypeTable();
    int SetupDefaultSubtypeTable();
    int SetupDefaultSourceTable();

    int PrepareStatements();
    int SetupDefaultBlackLibraryUsers();

    int BeginTransaction() const;
    int CheckInitialized() const;
    int EndTransaction() const;
    int GenerateTable(const std::string &sql);
    int PrepareStatement(const std::string &statement, int statement_id);
    int ResetStatement(sqlite3_stmt *smt) const;

    int BindInt(sqlite3_stmt* stmt, const std::string &parameter_name, const int &bind_int) const;
    int BindText(sqlite3_stmt* stmt, const std::string &parameter_name, const std::string &bind_text) const;

    int LogTraceStatement(sqlite3_stmt* stmt) const;

    sqlite3 *database_conn_;
    std::vector<sqlite3_stmt *> prepared_statements_;
    bool initialized_;
};

} // namespace db
} // namespace core
} // namespace black_library

#endif
