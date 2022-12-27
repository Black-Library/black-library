/**
 * SQLiteDB.cc
 */

#include <iostream>
#include <string>
#include <sstream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <SourceInformation.h>

#include <DBConnectionInterfaceUtils.h>
#include <SQLiteDB.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

static constexpr const char CreateUserTable[]                     = "CREATE TABLE IF NOT EXISTS user(UID INTEGER PRIMARY KEY, permission_level INTEGER DEFAULT 0 NOT NULL, name TEXT NOT NULL)";
static constexpr const char CreateMediaTypeTable[]                = "CREATE TABLE IF NOT EXISTS media_type(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateMediaSubtypeTable[]             = "CREATE TABLE IF NOT EXISTS media_subtype(name TEXT NOT NULL PRIMARY KEY, media_type_name TEXT, FOREIGN KEY(media_type_name) REFERENCES media_type(name))";
static constexpr const char CreateBookGenreTable[]                = "CREATE TABLE IF NOT EXISTS book_genre(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateDocumentTagTable[]              = "CREATE TABLE IF NOT EXISTS document_tag(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateSourceTable[]                   = "CREATE TABLE IF NOT EXISTS source(name TEXT NOT NULL PRIMARY KEY, media_type TEXT, media_subtype TEXT, FOREIGN KEY(media_type) REFERENCES media_type(name) FOREIGN KEY(media_subtype) REFERENCES media_subtype(name))";
static constexpr const char CreateStagingEntryTable[]             = "CREATE TABLE IF NOT EXISTS staging_entry(UUID VARCHAR(36) PRIMARY KEY NOT NULL, title TEXT NOT NULL, author TEXT NOT NULL, nickname TEXT, source TEXT, url TEXT, last_url TEXT, series TEXT, series_length DEFAULT 1, version INTEGER, media_path TEXT NOT NULL, birth_date INTEGER, check_date INTEGER, update_date INTEGER, user_contributed INTEGER NOT NULL, FOREIGN KEY(source) REFERENCES source(name), FOREIGN KEY(user_contributed) REFERENCES user(UID))";
static constexpr const char CreateBlackEntryTable[]               = "CREATE TABLE IF NOT EXISTS black_entry(UUID VARCHAR(36) PRIMARY KEY NOT NULL, title TEXT NOT NULL, author TEXT NOT NULL, nickname TEXT, source TEXT, url TEXT, last_url TEXT, series TEXT, series_length DEFAULT 1, version INTEGER, media_path TEXT NOT NULL, birth_date INTEGER, check_date INTEGER, update_date INTEGER, user_contributed INTEGER NOT NULL, FOREIGN KEY(source) REFERENCES source(name), FOREIGN KEY(user_contributed) REFERENCES user(UID))";
static constexpr const char CreateMd5SumTable[]                   = "CREATE TABLE IF NOT EXISTS md5_sum(UUID VARCHAR(36) NOT NULL, index_num INTERGER, md5_sum VARCHAR(32), version_num INTERGER, PRIMARY KEY (UUID, index_num)";
static constexpr const char CreateRefreshTable[]                  = "CREATE TABLE IF NOT EXISTS refresh(UUID VARCHAR(36) PRIMARY KEY NOT NULL, refresh_date INTERGER)";
static constexpr const char CreateErrorEntryTable[]               = "CREATE TABLE IF NOT EXISTS error_entry(UUID VARCHAR(36) PRIMARY KEY NOT NULL, progress_num INTEGER)";

static constexpr const char CreateUserStatement[]                 = "INSERT INTO user(UID, permission_level, name) VALUES (:UID, :permission_level, :name)";
static constexpr const char CreateMediaTypeStatement[]            = "INSERT INTO media_type(name) VALUES (:name)";
static constexpr const char CreateMediaSubtypeStatement[]         = "INSERT INTO media_subtype(name, media_type_name) VALUES (:name, :media_type_name)";
static constexpr const char CreateSourceStatement[]               = "INSERT INTO source(name, media_type, media_subtype) VALUES (:name, :media_type, :media_subtype)";
static constexpr const char CreateStagingEntryStatement[]         = "INSERT INTO staging_entry(UUID, title, author, nickname, source, url, last_url, series, series_length, version, media_path, birth_date, check_date, update_date, user_contributed) VALUES (:UUID, :title, :author, :nickname, :source, :url, :last_url, :series, :series_length, :version, :media_path, :birth_date, :check_date, :update_date, :user_contributed)";
static constexpr const char CreateBlackEntryStatement[]           = "INSERT INTO black_entry(UUID, title, author, nickname, source, url, last_url, series, series_length, version, media_path, birth_date, check_date, update_date, user_contributed) VALUES (:UUID, :title, :author, :nickname, :source, :url, :last_url, :series, :series_length, :version, :media_path, :birth_date, :check_date, :update_date, :user_contributed)";
static constexpr const char CreateMd5SumStatement[]               = "INSERT INTO md5_sum(UUID, index_num, md5_sum, version_num) VALUES (:UUID, :index_num, :md5_sum, :version_num)";
static constexpr const char CreateRefreshStatement[]              = "INSERT INTO refresh(UUID, refresh_date) VALUES (:UUID, :refresh_date)";
static constexpr const char CreateErrorEntryStatement[]           = "INSERT INTO error_entry(UUID, progress_num) VALUES (:UUID, :progress_num)";

static constexpr const char ReadStagingEntryStatement[]           = "SELECT * FROM staging_entry WHERE UUID = :UUID";
static constexpr const char ReadStagingEntryUrlStatement[]        = "SELECT * FROM staging_entry WHERE url = :url";
static constexpr const char ReadStagingEntryUUIDStatement[]       = "SELECT * FROM staging_entry WHERE UUID = :UUID";
static constexpr const char ReadBlackEntryStatement[]             = "SELECT * FROM black_entry WHERE UUID = :UUID";
static constexpr const char ReadBlackEntryUrlStatement[]          = "SELECT * FROM black_entry WHERE url = :url";
static constexpr const char ReadBlackEntryUUIDStatement[]         = "SELECT * FROM black_entry WHERE UUID = :UUID";
static constexpr const char ReadMd5SumStatement[]                 = "SELECT * FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char ReadRefreshStatement[]                = "SELECT * FROM refresh WHERE UUID = :UUID";
static constexpr const char ReadErrorEntryStatement[]             = "SELECT * FROM error_entry WHERE UUID = :UUID AND progress_num = :progress_num";

static constexpr const char UpdateStagingEntryStatement[]         = "UPDATE staging_entry SET title = :title, author = :author, nickname = :nickname, source = :source, url = :url, last_url = :last_url, series = :series, series_length = :series_length, version = :version, media_path = :media_path, birth_date = :birth_date, check_date = :check_date, update_date = :update_date, user_contributed = :user_contributed WHERE UUID = :UUID";
static constexpr const char UpdateBlackEntryStatement[]           = "UPDATE black_entry SET title = :title, author = :author, nickname = :nickname, source = :source, url = :url, last_url = :last_url, series = :series, series_length = :series_length, version = :version, media_path = :media_path, birth_date = :birth_date, check_date = :check_date, update_date = :update_date, user_contributed = :user_contributed WHERE UUID = :UUID";
static constexpr const char UpdateMd5SumStatement[]               = "UPDATE md5_sum SET md5_sum = :md5_sum, version_num = :version_num WHERE UUID = :UUID AND index_num = :index_num";

static constexpr const char DeleteStagingEntryStatement[]         = "DELETE FROM staging_entry WHERE UUID = :UUID";
static constexpr const char DeleteBlackEntryStatement[]           = "DELETE FROM black_entry WHERE UUID = :UUID";
static constexpr const char DeleteMd5SumStatement[]               = "DELETE FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char DeleteRefreshStatement[]              = "DELETE FROM refresh WHERE UUID = :UUID";
static constexpr const char DeleteErrorEntryStatement[]           = "DELETE FROM error_entry WHERE UUID = :UUID AND progress_num = :progress_num";

static constexpr const char GetStagingEntriesStatement[]          = "SELECT * FROM staging_entry";
static constexpr const char GetBlackEntriesStatement[]            = "SELECT * FROM black_entry";
static constexpr const char GetMd5sumsStatement[]                 = "SELECT * FROM md5_sum";
static constexpr const char GetErrorEntriesStatement[]            = "SELECT * FROM error_entry";

static constexpr const char DoesMinRefreshExistStatement[]        = "SELECT CASE WHEN EXISTS(SELECT 1 FROM refresh) THEN 1 ELSE 0 END";
static constexpr const char GetStagingEntryUUIDFromUrlStatement[] = "SELECT UUID FROM staging_entry WHERE url = :url";
static constexpr const char GetBlackEntryUUIDFromUrlStatement[]   = "SELECT UUID FROM black_entry WHERE url = :url";
static constexpr const char GetStagingEntryUrlFromUUIDStatement[] = "SELECT url, last_url FROM staging_entry WHERE UUID = :UUID";
static constexpr const char GetBlackEntryUrlFromUUIDStatement[]   = "SELECT url, last_url FROM black_entry WHERE UUID = :UUID";
static constexpr const char GetMd5SumFromUUIDAndIndexStatement[]  = "SELECT md5_sum FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char GetRefreshFromMinDateStatement[]      = "SELECT * FROM refresh WHERE refresh_date=(SELECT MIN(refresh_date) FROM refresh)";

typedef enum {
    CREATE_USER_STATEMENT,
    CREATE_MEDIA_TYPE_STATEMENT,
    CREATE_MEDIA_SUBTYPE_STATEMENT,
    CREATE_SOURCE_STATEMENT,
    CREATE_STAGING_ENTRY_STATEMENT,
    CREATE_BLACK_ENTRY_STATEMENT,
    CREATE_MD5_SUM_STATEMENT,
    CREATE_REFRESH_STATEMENT,
    CREATE_ERROR_ENTRY_STATEMENT,

    READ_STAGING_ENTRY_STATEMENT,
    READ_STAGING_ENTRY_URL_STATEMENT,
    READ_STAGING_ENTRY_UUID_STATEMENT,
    READ_BLACK_ENTRY_STATEMENT,
    READ_BLACK_ENTRY_URL_STATEMENT,
    READ_BLACK_ENTRY_UUID_STATEMENT,
    READ_MD5_SUM_STATEMENT,
    READ_REFRESH_STATEMENT,
    READ_ERROR_ENTRY_STATEMENT,

    UPDATE_STAGING_ENTRY_STATEMENT,
    UPDATE_BLACK_ENTRY_STATEMENT,
    UPDATE_MD5_SUM_STATEMENT,

    DELETE_STAGING_ENTRY_STATEMENT,
    DELETE_BLACK_ENTRY_STATEMENT,
    DELETE_MD5_SUM_STATEMENT,
    DELETE_REFRESH_STATEMENT,
    DELETE_ERROR_ENTRY_STATEMENT,

    GET_STAGING_ENTRIES_STATEMENT,
    GET_BLACK_ENTRIES_STATEMENT,
    GET_CHECKSUMS_STATEMENT,
    GET_ERROR_ENTRIES_STATEMENT,

    DOES_MIN_REFRESH_EXIST_STATEMENT,
    GET_STAGING_ENTRY_UUID_FROM_URL_STATEMENT,
    GET_STAGING_ENTRY_URL_FROM_UUID_STATEMENT,
    GET_BLACK_ENTRY_UUID_FROM_URL_STATEMENT,
    GET_BLACK_ENTRY_URL_FROM_UUID_STATEMENT,
    GET_MD5_SUM_FROM_UUID_AND_INDEX_STATEMENT,
    GET_REFRESH_FROM_MIN_DATE_STATEMENT,

    _NUM_PREPARED_STATEMENTS
} prepared_statement_id_t;

SQLiteDB::SQLiteDB(const std::string &database_url) :
    database_conn_(),
    prepared_statements_(),
    initialized_(false)
{
    std::string target_url = database_url;
    if (target_url.empty())
    {
        target_url = DefaultDBPath;
        BlackLibraryCommon::LogDebug("db", "Empty database url given, using default: {}", target_url);
    }

    bool first_time_setup = false;
    if (!BlackLibraryCommon::PathExists(target_url))
    {
        BlackLibraryCommon::LogDebug("db", "{} does not exist, first tiem setup enabled", target_url);
        first_time_setup = true;
    }

    int res = sqlite3_open(target_url.c_str(), &database_conn_);
    
    if (res != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Failed to open db at: {} - {}", target_url, sqlite3_errmsg(database_conn_));
        return;
    }

    BlackLibraryCommon::LogInfo("db", "Open database at: {}", target_url);

    if (first_time_setup)
    {
        if (GenerateTables())
        {
            BlackLibraryCommon::LogError("db", "Failed to generate database tables");
            return;
        }
    }

    if (PrepareStatements())
    {
        BlackLibraryCommon::LogError("db", "Failed to setup prepare statements");
        return;
    }

    if (first_time_setup)
    {
        if (SetupDefaultBlackLibraryUsers())
        {
            BlackLibraryCommon::LogError("db", "Failed to setup default black library users");
            return;
        }
        if (SetupDefaultTypeTables())
        {
            BlackLibraryCommon::LogError("db", "Failed to setup default type tables");
            return;
        }
    }

    initialized_ = true;
}

SQLiteDB::~SQLiteDB()
{
    if (database_conn_)
    {
        for (size_t i = 0; i < prepared_statements_.size(); ++i)
        {
            sqlite3_finalize(prepared_statements_[i]);
        }
        sqlite3_close(database_conn_);
    }
}

std::vector<DBEntry> SQLiteDB::ListEntries(entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "List {} entries", GetEntryTypeString(entry_type));

    std::vector<DBEntry> entries;

    if (CheckInitialized())
        return entries;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = GET_BLACK_ENTRIES_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = GET_STAGING_ENTRIES_STATEMENT;
            break;
        default:
            return entries;
    }

    if (BeginTransaction())
        return entries;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBEntry entry;

        entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        entry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        entry.author = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        entry.nickname = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        entry.source = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        entry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        entry.last_url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
        entry.series = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        entry.series_length = sqlite3_column_int(stmt, 8);
        entry.version = sqlite3_column_int(stmt, 9);
        entry.media_path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)));
        entry.birth_date = sqlite3_column_int(stmt, 11);
        entry.check_date = sqlite3_column_int(stmt, 12);
        entry.update_date = sqlite3_column_int(stmt, 13);
        entry.user_contributed = sqlite3_column_int(stmt, 14);

        entries.emplace_back(entry);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return entries;

    return entries;
}

std::vector<DBMd5Sum> SQLiteDB::ListChecksums() const
{
    BlackLibraryCommon::LogDebug("db", "List checksums");

    std::vector<DBMd5Sum> checksums;

    if (CheckInitialized())
        return checksums;

    if (BeginTransaction())
        return checksums;

    sqlite3_stmt *stmt = prepared_statements_[GET_CHECKSUMS_STATEMENT];

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBMd5Sum checksum;

        checksum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        checksum.index_num = sqlite3_column_int(stmt, 1);
        checksum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        checksum.version_num = sqlite3_column_int(stmt, 3);

        checksums.emplace_back(checksum);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return checksums;

    return checksums;
}

std::vector<DBErrorEntry> SQLiteDB::ListErrorEntries() const
{
    BlackLibraryCommon::LogDebug("db", "List error entries");

    std::vector<DBErrorEntry> entries;

    if (CheckInitialized())
        return entries;

    if (BeginTransaction())
        return entries;

    sqlite3_stmt *stmt = prepared_statements_[GET_ERROR_ENTRIES_STATEMENT];

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBErrorEntry entry;

        entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        entry.progress_num = sqlite3_column_int(stmt, 1);

        entries.emplace_back(entry);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return entries;

    return entries;
}

int SQLiteDB::CreateUser(const DBUser &user) const
{
    BlackLibraryCommon::LogDebug("db", "Create user: {} with UID: {}", user.name, user.uid);

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_USER_STATEMENT];

    // bind statement variables
    if (BindInt(stmt, "UID", user.uid))
        return -1;
    if (BindInt(stmt, "permission_level", static_cast<uint8_t>(user.permission_level)))
        return -1;
    if (BindText(stmt, "name", user.name))
        return -1;

    // run statement
    int ret = SQLITE_OK;

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create user: {} with UID: {} failed: {}", user.name, user.uid, sqlite3_errmsg(database_conn_));
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateMediaType(const std::string &media_type_name) const
{
    BlackLibraryCommon::LogDebug("db", "Create media type: {}", media_type_name);

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_MEDIA_TYPE_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "name", media_type_name))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create media type: {} failed: {}", media_type_name, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateMediaSubtype(const std::string &media_subtype_name, const std::string &media_type_name) const
{
    BlackLibraryCommon::LogDebug("db", "Create subtype: {} media: {}", media_subtype_name, media_type_name);

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_MEDIA_SUBTYPE_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "name", media_subtype_name))
        return -1;
    if (BindText(stmt, "media_type_name", media_type_name))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create subtype: {} - media: {} failed: {}", media_subtype_name, media_type_name, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateSource(const DBSource &source) const
{
    BlackLibraryCommon::LogDebug("db", "Create source: {}", source.name);

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_SOURCE_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "name", source.name))
        return -1;
    if (BindText(stmt, "media_type", GetMediaTypeString(source.media_type)))
        return -1;
    if (BindText(stmt, "media_subtype", GetMediaSubtypeString(source.subtype)))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create source: {} failed: {}", source.name, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateEntry(const DBEntry &entry, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Create {} entry with UUID: {}", GetEntryTypeString(entry_type), entry.uuid);

    if (CheckInitialized())
        return -1;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = CREATE_BLACK_ENTRY_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = CREATE_STAGING_ENTRY_STATEMENT;
            break;
        default:
            return -1;
    }

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", entry.uuid))
        return -1;
    if (BindText(stmt, "title", entry.title))
        return -1;
    if (BindText(stmt, "author", entry.author))
        return -1;
    if (BindText(stmt, "nickname", entry.nickname))
        return -1;
    if (BindText(stmt, "source", entry.source))
        return -1;
    if (BindText(stmt, "url", entry.url))
        return -1;
    if (BindText(stmt, "last_url", entry.last_url))
        return -1;
    if (BindText(stmt, "series", entry.series))
        return -1;
    if (BindInt(stmt, "series_length", entry.series_length))
        return -1;
    if (BindInt(stmt, "version", entry.version))
        return -1;
    if (BindText(stmt, "media_path", entry.media_path))
        return -1;
    if (BindInt(stmt, "birth_date", entry.birth_date))
        return -1;
    if (BindInt(stmt, "check_date", entry.check_date))
        return -1;
    if (BindInt(stmt, "update_date", entry.update_date))
        return -1;
    if (BindInt(stmt, "user_contributed", entry.user_contributed))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create {} entry failed: {}", GetEntryTypeString(entry_type), sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

DBEntry SQLiteDB::ReadEntry(const std::string &uuid, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Read {} entry with UUID: {}", GetEntryTypeString(entry_type), uuid);

    DBEntry entry;

    if (CheckInitialized())
        return entry;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = READ_BLACK_ENTRY_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = READ_STAGING_ENTRY_STATEMENT;
            break;
        default:
            return entry;
    }

    if (BeginTransaction())
        return entry;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return entry;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError("db", "Read {} entry failed: {}", GetEntryTypeString(entry_type), sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return entry;
    }

    entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    entry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    entry.author = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    entry.nickname = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
    entry.source = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
    entry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
    entry.last_url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
    entry.series = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
    entry.series_length = sqlite3_column_int(stmt, 8);
    entry.version = sqlite3_column_int(stmt, 9);
    entry.media_path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)));
    entry.birth_date = sqlite3_column_int(stmt, 11);
    entry.check_date = sqlite3_column_int(stmt, 12);
    entry.update_date = sqlite3_column_int(stmt, 13);
    entry.user_contributed = sqlite3_column_int(stmt, 14);

    ResetStatement(stmt);

    if (EndTransaction())
        return entry;

    return entry;
}

int SQLiteDB::UpdateEntry(const DBEntry &entry, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Update {} entry with UUID: {}", GetEntryTypeString(entry_type), entry.uuid);

    if (CheckInitialized())
        return -1;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = UPDATE_BLACK_ENTRY_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = UPDATE_STAGING_ENTRY_STATEMENT;
            break;
        default:
            return -1;
    }

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", entry.uuid))
        return -1;
    if (BindText(stmt, "title", entry.title))
        return -1;
    if (BindText(stmt, "author", entry.author))
        return -1;
    if (BindText(stmt, "nickname", entry.nickname))
        return -1;
    if (BindText(stmt, "source", entry.source))
        return -1;
    if (BindText(stmt, "url", entry.url))
        return -1;
    if (BindText(stmt, "last_url", entry.last_url))
        return -1;
    if (BindText(stmt, "series", entry.series))
        return -1;
    if (BindInt(stmt, "series_length", entry.series_length))
        return -1;
    if (BindInt(stmt, "version", entry.version))
        return -1;
    if (BindText(stmt, "media_path", entry.media_path))
        return -1;
    if (BindInt(stmt, "birth_date", entry.birth_date))
        return -1;
    if (BindInt(stmt, "check_date", entry.check_date))
        return -1;
    if (BindInt(stmt, "update_date", entry.update_date))
        return -1;
    if (BindInt(stmt, "user_contributed", entry.user_contributed))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Update {} entry failed: {}", GetEntryTypeString(entry_type), sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::DeleteEntry(const std::string &uuid, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Delete {} entry with UUID: {}", GetEntryTypeString(entry_type), uuid);

    if (CheckInitialized())
        return -1;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = DELETE_BLACK_ENTRY_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = DELETE_STAGING_ENTRY_STATEMENT;
            break;
        default:
            return -1;
    }

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Delete {} entry failed: {}", GetEntryTypeString(entry_type), sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateMd5Sum(const DBMd5Sum &md5) const
{
    BlackLibraryCommon::LogDebug("db", "Create MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", md5.uuid))
        return -1;
    if (BindInt(stmt, "index_num", md5.index_num))
        return -1;
    if (BindText(stmt, "md5_sum", md5.md5_sum))
        return -1;
    if (BindInt(stmt, "version_num", md5.version_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

DBMd5Sum SQLiteDB::ReadMd5Sum(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug("db", "Read MD5 checksum with UUID: {} index_num: {}", uuid, index_num);

    DBMd5Sum md5;

    if (CheckInitialized())
        return md5;

    if (BeginTransaction())
        return md5;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return md5;
    if (BindInt(stmt, "index_num", index_num))
        return md5;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError("db", "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return md5;
    }

    md5.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    md5.index_num = sqlite3_column_int(stmt, 1);
    md5.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    md5.version_num = sqlite3_column_int(stmt, 3);

    ResetStatement(stmt);

    if (EndTransaction())
        return md5;

    return md5;
}

int SQLiteDB::UpdateMd5Sum(const DBMd5Sum &md5) const
{
    BlackLibraryCommon::LogDebug("db", "Update MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[UPDATE_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", md5.uuid))
        return -1;
    if (BindInt(stmt, "index_num", md5.index_num))
        return -1;
    if (BindText(stmt, "md5_sum", md5.md5_sum))
        return -1;
    if (BindInt(stmt, "version_num", md5.version_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Update MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::DeleteMd5Sum(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug("db", "Delete MD5 checksum with UUID: {} index_num: {}", uuid, index_num);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[DELETE_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return -1;
    if (BindInt(stmt, "index_num", index_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Delete MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateRefresh(const DBRefresh &refresh) const
{
    BlackLibraryCommon::LogDebug("db", "Create refresh with UUID: {} refresh_date: {}", refresh.uuid, refresh.refresh_date);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_REFRESH_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", refresh.uuid))
        return -1;
    if (BindInt(stmt, "refresh_date", refresh.refresh_date))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create refresh checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;
    
    return 0;
}

DBRefresh SQLiteDB::ReadRefresh(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug("db", "Read refresh with UUID: {}", uuid);

    DBRefresh refresh;

    if (CheckInitialized())
        return refresh;

    if (BeginTransaction())
        return refresh;

    sqlite3_stmt *stmt = prepared_statements_[READ_REFRESH_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return refresh;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError("db", "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return refresh;
    }

    refresh.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    refresh.refresh_date = sqlite3_column_int(stmt, 1);

    ResetStatement(stmt);

    if (EndTransaction())
        return refresh;

    return refresh;
}

int SQLiteDB::DeleteRefresh(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug("db", "Delete refresh with UUID: {}", uuid);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[DELETE_REFRESH_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Delete refresh failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateErrorEntry(const DBErrorEntry &entry) const
{
    BlackLibraryCommon::LogDebug("db", "Create error entry for UUID: {}", entry.uuid);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_ERROR_ENTRY_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", entry.uuid))
        return -1;
    if (BindInt(stmt, "progress_num", entry.progress_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Create error entry for UUID: {} failed: {}", entry.uuid, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::DeleteErrorEntry(const std::string &uuid, size_t progress_num) const
{
    BlackLibraryCommon::LogDebug("db", "Delete error entry for UUID: {} and progress number: {}", uuid, progress_num);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[DELETE_ERROR_ENTRY_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return -1;
    if (BindInt(stmt, "progress_num", progress_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError("db", "Delete error entry for UUID: {} and progress number: {} failed: {}", uuid, progress_num, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

DBBoolResult SQLiteDB::DoesEntryUrlExist(const std::string &url, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Check {} entries for url: {}", GetEntryTypeString(entry_type), url);

    DBBoolResult check;

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = READ_BLACK_ENTRY_URL_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = READ_STAGING_ENTRY_URL_STATEMENT;
            break;
        default:
            return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "url", url))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "Entry {} url: {} does not exist", GetEntryTypeString(entry_type), url);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }
    else
    {
        check.result = true;
    }
    

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesEntryUUIDExist(const std::string &uuid, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Check {} entries for UUID: {}", GetEntryTypeString(entry_type), uuid);

    DBBoolResult check;

    if (uuid.empty())
    {
        check.result = false;
        return check;
    }

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = READ_BLACK_ENTRY_UUID_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = READ_STAGING_ENTRY_UUID_STATEMENT;
            break;
        default:
            return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "Entry {} UUID: {} does not exist",  GetEntryTypeString(entry_type), uuid);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }
    else
    {
        check.result = true;
    }
    

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesMd5SumExist(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug("db", "Check MD5 checksum for UUID: {}, index_num: {}", uuid, index_num);

    DBBoolResult check;

    if (uuid.empty())
    {
        check.result = false;
        return check;
    }

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }
    if (BindInt(stmt, "index_num", index_num))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "MD5 checksum UUID: {} index_num: {} does not exist", uuid, index_num);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }
    else
    {
        check.result = true;
    }

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesRefreshExist(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug("db", "Check refresh for UUID: {}", uuid);

    DBBoolResult check;

    if (uuid.empty())
    {
        check.result = false;
        return check;
    }

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[READ_REFRESH_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "refresh UUID: {} does not exist", uuid);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }
    else
    {
        check.result = true;
    }

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesErrorEntryExist(const std::string &uuid, size_t progress_num) const
{
    BlackLibraryCommon::LogDebug("db", "Check error entries for UUID: {}, progress_num: {}", uuid, progress_num);

    DBBoolResult check;

    if (uuid.empty())
    {
        check.result = false;
        return check;
    }

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[READ_ERROR_ENTRY_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }
    if (BindInt(stmt, "progress_num", progress_num))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "UUID: {} progress_num: {} does not exist", uuid, progress_num);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }
    else
    {
        check.result = true;
    }

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesMinRefreshExist() const
{
    BlackLibraryCommon::LogDebug("db", "Check refresh for min date");

    DBBoolResult check;

    if (CheckInitialized())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    if (BeginTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    sqlite3_stmt *stmt = prepared_statements_[DOES_MIN_REFRESH_EXIST_STATEMENT];

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "refresh does not exist");
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }

    check.result = sqlite3_column_int(stmt, 0);

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBStringResult SQLiteDB::GetEntryUUIDFromUrl(const std::string &url, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Get UUID from url: {}", url);

    DBStringResult res;

    if (CheckInitialized())
        return res;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = GET_BLACK_ENTRY_UUID_FROM_URL_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = GET_STAGING_ENTRY_UUID_FROM_URL_STATEMENT;
            break;
        default:
            res.error = true;
            return res;
    }

    if (BeginTransaction())
        return res;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "url", url))
        return res;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "Url: {} does not exist", url);
        ResetStatement(stmt);
        EndTransaction();
        res.does_not_exist = true;
        res.error = false;
        return res;
    }

    res.result = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

    ResetStatement(stmt);

    if (EndTransaction())
        return res;

    res.error = false;

    return res;
}

DBStringResult SQLiteDB::GetEntryUrlFromUUID(const std::string &uuid, entry_table_rep_t entry_type) const
{
    BlackLibraryCommon::LogDebug("db", "Get url from UUID: {}", uuid);

    DBStringResult res;

    if (CheckInitialized())
        return res;

    int statement_id;
    switch (entry_type)
    {
        case BLACK_ENTRY:
            statement_id = GET_BLACK_ENTRY_URL_FROM_UUID_STATEMENT;
            break;
        case STAGING_ENTRY:
            statement_id = GET_STAGING_ENTRY_URL_FROM_UUID_STATEMENT;
            break;
        default:
            EndTransaction();
            res.error = true;
            return res;
    }

    if (BeginTransaction())
        return res;

    sqlite3_stmt *stmt = prepared_statements_[statement_id];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return res;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug("db", "UUID: {} does not exist", uuid);
        ResetStatement(stmt);
        EndTransaction();
        res.does_not_exist = true;
        res.error = false;
        return res;
    }

    std::string url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    std::string last_url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

    ResetStatement(stmt);

    if (EndTransaction())
        return res;

    res.result = url;

    res.error = false;

    return res;
}

uint16_t SQLiteDB::GetVersionFromMd5(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug("db", "Get version from MD5");

    uint16_t version_num = 0;

    if (CheckInitialized())
        return version_num;

    if (BeginTransaction())
        return version_num;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return version_num;
    if (BindInt(stmt, "index_num", index_num))
        return version_num;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError("db", "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return version_num;
    }

    version_num = sqlite3_column_int(stmt, 3);

    ResetStatement(stmt);

    if (EndTransaction())
        return version_num;

    return version_num;
}

DBRefresh SQLiteDB::GetRefreshFromMinDate() const
{
    BlackLibraryCommon::LogDebug("db", "Get refresh from next date");

    DBRefresh refresh;

    if (CheckInitialized())
        return refresh;

    if (BeginTransaction())
        return refresh;

    sqlite3_stmt *stmt = prepared_statements_[GET_REFRESH_FROM_MIN_DATE_STATEMENT];

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError("db", "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return refresh;
    }

    refresh.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    refresh.refresh_date = sqlite3_column_int(stmt, 1);

    ResetStatement(stmt);

    if (EndTransaction())
        return refresh;

    return refresh;
}

bool SQLiteDB::IsReady() const
{
    return initialized_;
}

int SQLiteDB::GenerateTables()
{
    BlackLibraryCommon::LogDebug("db", "Setting up tables");

    int res = 0;

    res += GenerateTable(CreateUserTable);
    res += GenerateTable(CreateMediaTypeTable);
    res += GenerateTable(CreateMediaSubtypeTable);
    res += GenerateTable(CreateBookGenreTable);
    res += GenerateTable(CreateDocumentTagTable);
    res += GenerateTable(CreateSourceTable);
    res += GenerateTable(CreateStagingEntryTable);
    res += GenerateTable(CreateBlackEntryTable);
    res += GenerateTable(CreateMd5SumTable);
    res += GenerateTable(CreateRefreshTable);
    res += GenerateTable(CreateErrorEntryTable);

    return res;
}

int SQLiteDB::SetupDefaultTypeTables()
{
    BlackLibraryCommon::LogDebug("db", "Setting up default type tables");

    int res = 0;

    res += SetupDefaultEntryTypeTable();
    res += SetupDefaultSubtypeTable();
    res += SetupDefaultSourceTable();

    return res;
}

int SQLiteDB::SetupDefaultEntryTypeTable()
{
    int res = 0;

    res += CreateMediaType(GetMediaTypeString(DBEntryMediaType::Document));
    res += CreateMediaType(GetMediaTypeString(DBEntryMediaType::ImageGallery));
    res += CreateMediaType(GetMediaTypeString(DBEntryMediaType::Video));

    return res;
}

int SQLiteDB::SetupDefaultSubtypeTable()
{
    int res = 0;

    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::BLOG), GetMediaTypeString(DBEntryMediaType::Document));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::BOOK), GetMediaTypeString(DBEntryMediaType::Document));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::NEWS_ARTICLE), GetMediaTypeString(DBEntryMediaType::Document));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::PAPER), GetMediaTypeString(DBEntryMediaType::Document));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::WEBNOVEL), GetMediaTypeString(DBEntryMediaType::Document));

    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::MANGA), GetMediaTypeString(DBEntryMediaType::ImageGallery));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::PHOTO_ALBUM), GetMediaTypeString(DBEntryMediaType::ImageGallery));

    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::MOVIE), GetMediaTypeString(DBEntryMediaType::Video));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::TV_SHOW), GetMediaTypeString(DBEntryMediaType::Video));
    res += CreateMediaSubtype(GetMediaSubtypeString(DBEntryMediaSubtype::YOUTUBE), GetMediaTypeString(DBEntryMediaType::Video));

    return res;
}

int SQLiteDB::SetupDefaultSourceTable()
{
    DBSource ao3_source;
    DBSource ffn_source;
    DBSource rr_source;
    DBSource sbf_source;
    DBSource svf_source;
    DBSource yt_source;

    ao3_source.name = BlackLibraryCommon::AO3::source_name;
    ao3_source.media_type = DBEntryMediaType::Document;
    ao3_source.subtype = DBEntryMediaSubtype::WEBNOVEL;

    ffn_source.name = BlackLibraryCommon::FFN::source_name;
    ffn_source.media_type = DBEntryMediaType::Document;
    ffn_source.subtype = DBEntryMediaSubtype::WEBNOVEL;

    rr_source.name = BlackLibraryCommon::RR::source_name;
    rr_source.media_type = DBEntryMediaType::Document;
    rr_source.subtype = DBEntryMediaSubtype::WEBNOVEL;

    sbf_source.name = BlackLibraryCommon::SBF::source_name;
    sbf_source.media_type = DBEntryMediaType::Document;
    sbf_source.subtype = DBEntryMediaSubtype::WEBNOVEL;

    svf_source.name = BlackLibraryCommon::SVF::source_name;
    svf_source.media_type = DBEntryMediaType::Document;
    svf_source.subtype = DBEntryMediaSubtype::WEBNOVEL;

    yt_source.name = BlackLibraryCommon::YT::source_name;
    yt_source.media_type = DBEntryMediaType::Video;
    yt_source.subtype = DBEntryMediaSubtype::YOUTUBE;

    if (CreateSource(ao3_source))
        return -1;
    if (CreateSource(ffn_source))
        return -1;
    if (CreateSource(rr_source))
        return -1;
    if (CreateSource(sbf_source))
        return -1;
    if (CreateSource(svf_source))
        return -1;
    if (CreateSource(yt_source))
        return -1;

    return 0;
}

int SQLiteDB::PrepareStatements()
{
    if (!database_conn_)
        return -1;

    int res = 0;

    res += PrepareStatement(CreateUserStatement, CREATE_USER_STATEMENT);
    res += PrepareStatement(CreateMediaTypeStatement, CREATE_MEDIA_TYPE_STATEMENT);
    res += PrepareStatement(CreateMediaSubtypeStatement, CREATE_MEDIA_SUBTYPE_STATEMENT);
    res += PrepareStatement(CreateSourceStatement, CREATE_SOURCE_STATEMENT);
    res += PrepareStatement(CreateStagingEntryStatement, CREATE_STAGING_ENTRY_STATEMENT);
    res += PrepareStatement(CreateBlackEntryStatement, CREATE_BLACK_ENTRY_STATEMENT);
    res += PrepareStatement(CreateMd5SumStatement, CREATE_MD5_SUM_STATEMENT);
    res += PrepareStatement(CreateRefreshStatement, CREATE_REFRESH_STATEMENT);
    res += PrepareStatement(CreateErrorEntryStatement, CREATE_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(ReadStagingEntryStatement, READ_STAGING_ENTRY_STATEMENT);
    res += PrepareStatement(ReadStagingEntryUrlStatement, READ_STAGING_ENTRY_URL_STATEMENT);
    res += PrepareStatement(ReadStagingEntryUUIDStatement, READ_STAGING_ENTRY_UUID_STATEMENT);
    res += PrepareStatement(ReadBlackEntryStatement, READ_BLACK_ENTRY_STATEMENT);
    res += PrepareStatement(ReadBlackEntryUrlStatement, READ_BLACK_ENTRY_URL_STATEMENT);
    res += PrepareStatement(ReadBlackEntryUUIDStatement, READ_BLACK_ENTRY_UUID_STATEMENT);
    res += PrepareStatement(ReadMd5SumStatement, READ_MD5_SUM_STATEMENT);
    res += PrepareStatement(ReadRefreshStatement, READ_REFRESH_STATEMENT);
    res += PrepareStatement(ReadErrorEntryStatement, READ_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(UpdateStagingEntryStatement, UPDATE_STAGING_ENTRY_STATEMENT);
    res += PrepareStatement(UpdateBlackEntryStatement, UPDATE_BLACK_ENTRY_STATEMENT);
    res += PrepareStatement(UpdateMd5SumStatement, UPDATE_MD5_SUM_STATEMENT);

    res += PrepareStatement(DeleteStagingEntryStatement, DELETE_STAGING_ENTRY_STATEMENT);
    res += PrepareStatement(DeleteBlackEntryStatement, DELETE_BLACK_ENTRY_STATEMENT);
    res += PrepareStatement(DeleteMd5SumStatement, DELETE_MD5_SUM_STATEMENT);
    res += PrepareStatement(DeleteRefreshStatement, DELETE_REFRESH_STATEMENT);
    res += PrepareStatement(DeleteErrorEntryStatement, DELETE_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(GetStagingEntriesStatement, GET_STAGING_ENTRIES_STATEMENT);
    res += PrepareStatement(GetBlackEntriesStatement, GET_BLACK_ENTRIES_STATEMENT);
    res += PrepareStatement(GetMd5sumsStatement, GET_CHECKSUMS_STATEMENT);
    res += PrepareStatement(GetErrorEntriesStatement, GET_ERROR_ENTRIES_STATEMENT);

    res += PrepareStatement(DoesMinRefreshExistStatement, DOES_MIN_REFRESH_EXIST_STATEMENT);
    res += PrepareStatement(GetStagingEntryUUIDFromUrlStatement, GET_STAGING_ENTRY_UUID_FROM_URL_STATEMENT);
    res += PrepareStatement(GetStagingEntryUrlFromUUIDStatement, GET_STAGING_ENTRY_URL_FROM_UUID_STATEMENT);
    res += PrepareStatement(GetBlackEntryUUIDFromUrlStatement, GET_BLACK_ENTRY_UUID_FROM_URL_STATEMENT);
    res += PrepareStatement(GetBlackEntryUrlFromUUIDStatement, GET_BLACK_ENTRY_URL_FROM_UUID_STATEMENT);
    res += PrepareStatement(GetMd5SumFromUUIDAndIndexStatement, GET_MD5_SUM_FROM_UUID_AND_INDEX_STATEMENT);
    res += PrepareStatement(GetRefreshFromMinDateStatement, GET_REFRESH_FROM_MIN_DATE_STATEMENT);

    return res;
}

int SQLiteDB::SetupDefaultBlackLibraryUsers()
{
    DBUser black_library_admin;
    DBUser black_library_librarian;
    DBUser black_library_writer;
    DBUser black_library_reader;
    DBUser black_library_no_permissions;

    black_library_admin.uid = 4007;
    black_library_admin.permission_level = DBPermissions::ReadWriteExecutePermission;
    black_library_admin.name = "BlackLibraryAdmin";

    black_library_librarian.uid = 4004;
    black_library_librarian.permission_level = DBPermissions::ReadWritePermission;
    black_library_librarian.name = "BlackLibraryLibrarian";

    black_library_writer.uid = 4003;
    black_library_writer.permission_level = DBPermissions::WritePermission;
    black_library_writer.name = "BlackLibraryWriter";

    black_library_reader.uid = 4001;
    black_library_reader.permission_level = DBPermissions::ReadPermission;
    black_library_reader.name = "BlackLibraryReader";

    black_library_no_permissions.uid = 4000;
    black_library_no_permissions.permission_level = DBPermissions::NoPermission;
    black_library_no_permissions.name = "BlackLibraryNoPermissions";

    if (CreateUser(black_library_admin))
        return -1;
    if (CreateUser(black_library_librarian))
        return -1;
    if (CreateUser(black_library_writer))
        return -1;
    if (CreateUser(black_library_reader))
        return -1;
    if (CreateUser(black_library_no_permissions))
        return -1;

    return 0;
}

int SQLiteDB::BeginTransaction() const
{
    char *error_msg = 0;
    BlackLibraryCommon::LogTrace("db", "Begin transaction");
    int ret = sqlite3_exec(database_conn_, "BEGIN TRANSACTION", 0, 0, &error_msg);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Begin transaction failed: {} - {}", error_msg, sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::CheckInitialized() const
{
    if (!initialized_)
    {
        BlackLibraryCommon::LogError("db", "Database not initialized");
        return -1;
    }

    return 0;
}

int SQLiteDB::EndTransaction() const
{
    char *error_msg = 0;
    BlackLibraryCommon::LogTrace("db", "End transaction");
    int ret = sqlite3_exec(database_conn_, "END TRANSACTION", 0, 0, &error_msg);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "End transaction  failed: {} - {}", error_msg, sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::GenerateTable(const std::string &sql)
{
    char *error_msg = 0;
    int ret = sqlite3_exec(database_conn_, sql.c_str(), 0, 0, &error_msg);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Generate table failed: {}", sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

// TODO: fix this so it uses a map intead of memory mapping in order
int SQLiteDB::PrepareStatement(const std::string &statement, int statement_id)
{
    prepared_statements_.emplace_back();
    int ret = sqlite3_prepare_v2(database_conn_, statement.c_str(), -1, &prepared_statements_[statement_id], nullptr);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Prepare failed: {}", sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::ResetStatement(sqlite3_stmt* stmt) const
{
    int ret = sqlite3_reset(stmt);
    BlackLibraryCommon::LogTrace("db", "Reset statement");
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Reset statement failed: {}", sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::BindInt(sqlite3_stmt* stmt, const std::string &parameter_name, const int &bind_int) const
{
    BlackLibraryCommon::LogTrace("db", "BindInt parameter:{} with {}", parameter_name, bind_int);
    const std::string parameter_index_name = ":" + parameter_name;
    int ret = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, parameter_index_name.c_str()), bind_int);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Bind of {}: {} failed: {}", parameter_name, bind_int, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    return 0;
}

int SQLiteDB::BindText(sqlite3_stmt* stmt, const std::string &parameter_name, const std::string &bind_text) const
{
    BlackLibraryCommon::LogTrace("db", "BindText parameter:{} with {}", parameter_name, bind_text);
    const std::string parameter_index_name = ":" + parameter_name;
    int index = sqlite3_bind_parameter_index(stmt, parameter_index_name.c_str());
    int ret = sqlite3_bind_text(stmt, index, bind_text.c_str(), bind_text.length(), SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError("db", "Bind of {}: {} failed: {}", parameter_name, bind_text, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    return 0;
}

int SQLiteDB::LogTraceStatement(sqlite3_stmt* stmt) const
{
    char *trace_sql = sqlite3_expanded_sql(stmt);
    BlackLibraryCommon::LogTrace("db", "{}", std::string(trace_sql));
    sqlite3_free(trace_sql);

    return 0;
}

} // namespace db
} // namespace core
} // namespace black_library
