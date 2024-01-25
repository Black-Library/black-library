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

static constexpr const char CreateDBVersionTable[]                = "CREATE TABLE IF NOT EXISTS db_version(db_version TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateUserTable[]                     = "CREATE TABLE IF NOT EXISTS user(UID INTEGER PRIMARY KEY, permission_level INTEGER DEFAULT 0 NOT NULL, name TEXT NOT NULL)";
static constexpr const char CreateMediaTypeTable[]                = "CREATE TABLE IF NOT EXISTS media_type(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateMediaSubtypeTable[]             = "CREATE TABLE IF NOT EXISTS media_subtype(name TEXT NOT NULL PRIMARY KEY, media_type_name TEXT, FOREIGN KEY(media_type_name) REFERENCES media_type(name))";
static constexpr const char CreateBookGenreTable[]                = "CREATE TABLE IF NOT EXISTS book_genre(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateDocumentTagTable[]              = "CREATE TABLE IF NOT EXISTS document_tag(name TEXT NOT NULL PRIMARY KEY)";
static constexpr const char CreateSourceTable[]                   = "CREATE TABLE IF NOT EXISTS source(name TEXT NOT NULL PRIMARY KEY, media_type TEXT, media_subtype TEXT, FOREIGN KEY(media_type) REFERENCES media_type(name) FOREIGN KEY(media_subtype) REFERENCES media_subtype(name))";
static constexpr const char CreateWorkEntryTable[]                = "CREATE TABLE IF NOT EXISTS work_entry(UUID VARCHAR(36) NOT NULL PRIMARY KEY, title TEXT NOT NULL, author TEXT NOT NULL, nickname TEXT, source TEXT, url TEXT, last_url TEXT, series TEXT, series_length DEFAULT 1, version INTEGER, media_path TEXT NOT NULL, birth_date INTEGER, check_date INTEGER, update_date INTEGER, user_contributed INTEGER NOT NULL, processing INTEGER NOT NULL, FOREIGN KEY(source) REFERENCES source(name), FOREIGN KEY(user_contributed) REFERENCES user(UID))";
static constexpr const char CreateMd5SumTable[]                   = "CREATE TABLE IF NOT EXISTS md5_sum(UUID VARCHAR(36) NOT NULL, index_num INTERGER, md5_sum VARCHAR(32), date INTERGER, sec_id TEXT, seq_num INTERGER, version_num INTERGER, PRIMARY KEY (UUID, index_num), FOREIGN KEY(UUID) REFERENCES work_entry(UUID))";
static constexpr const char CreateRefreshTable[]                  = "CREATE TABLE IF NOT EXISTS refresh(UUID VARCHAR(36) NOT NULL PRIMARY KEY, refresh_date INTERGER, FOREIGN KEY(UUID) REFERENCES work_entry(UUID))";
static constexpr const char CreateErrorEntryTable[]               = "CREATE TABLE IF NOT EXISTS error_entry(UUID VARCHAR(36) NOT NULL PRIMARY KEY, progress_num INTEGER, FOREIGN KEY(UUID) REFERENCES work_entry(UUID))";

static constexpr const char CreateDBVersionStatement[]            = "INSERT INTO db_version(db_version) VALUES (:db_version)";
static constexpr const char CreateUserStatement[]                 = "INSERT INTO user(UID, permission_level, name) VALUES (:UID, :permission_level, :name)";
static constexpr const char CreateMediaTypeStatement[]            = "INSERT INTO media_type(name) VALUES (:name)";
static constexpr const char CreateMediaSubtypeStatement[]         = "INSERT INTO media_subtype(name, media_type_name) VALUES (:name, :media_type_name)";
static constexpr const char CreateSourceStatement[]               = "INSERT INTO source(name, media_type, media_subtype) VALUES (:name, :media_type, :media_subtype)";
static constexpr const char CreateWorkEntryStatement[]            = "INSERT INTO work_entry(UUID, title, author, nickname, source, url, last_url, series, series_length, version, media_path, birth_date, check_date, update_date, user_contributed, processing) VALUES (:UUID, :title, :author, :nickname, :source, :url, :last_url, :series, :series_length, :version, :media_path, :birth_date, :check_date, :update_date, :user_contributed, :processing)";
static constexpr const char CreateMd5SumStatement[]               = "INSERT INTO md5_sum(UUID, index_num, md5_sum, date, sec_id, seq_num, version_num) VALUES (:UUID, :index_num, :md5_sum, :date, :sec_id, :seq_num, :version_num)";
static constexpr const char CreateRefreshStatement[]              = "INSERT INTO refresh(UUID, refresh_date) VALUES (:UUID, :refresh_date)";
static constexpr const char CreateErrorEntryStatement[]           = "INSERT INTO error_entry(UUID, progress_num) VALUES (:UUID, :progress_num)";

static constexpr const char ReadWorkEntryStatement[]              = "SELECT * FROM work_entry WHERE UUID = :UUID";
static constexpr const char ReadWorkEntryUrlStatement[]           = "SELECT * FROM work_entry WHERE url = :url";
static constexpr const char ReadWorkEntryUUIDStatement[]          = "SELECT * FROM work_entry WHERE UUID = :UUID";
static constexpr const char ReadMd5SumByIndexNumStatement[]       = "SELECT * FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char ReadMd5SumBySecIdStatement[]          = "SELECT * FROM md5_sum WHERE UUID = :UUID AND sec_id = :sec_id";
static constexpr const char ReadMd5SumBySeqNumStatement[]         = "SELECT * FROM md5_sum WHERE UUID = :UUID AND seq_num = :seq_num";
static constexpr const char ReadRefreshStatement[]                = "SELECT * FROM refresh WHERE UUID = :UUID";
static constexpr const char ReadErrorEntryStatement[]             = "SELECT * FROM error_entry WHERE UUID = :UUID AND progress_num = :progress_num";

static constexpr const char UpdateWorkEntryStatement[]            = "UPDATE work_entry SET title = :title, author = :author, nickname = :nickname, source = :source, url = :url, last_url = :last_url, series = :series, series_length = :series_length, version = :version, media_path = :media_path, birth_date = :birth_date, check_date = :check_date, update_date = :update_date, user_contributed = :user_contributed, processing = :processing WHERE UUID = :UUID";
static constexpr const char UpdateMd5SumStatement[]               = "UPDATE md5_sum SET md5_sum = :md5_sum, date = :date, sec_id = :sec_id, seq_num = :seq_num, version_num = :version_num WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char UpdateMd5SumStatementBySeqNum[]       = "UPDATE md5_sum SET md5_sum = :md5_sum, date = :date, sec_id = :sec_id, seq_num = :seq_num, version_num = :version_num WHERE UUID = :UUID AND seq_num = :seq_num";

static constexpr const char DeleteWorkEntryStatement[]            = "DELETE FROM work_entry WHERE UUID = :UUID";
static constexpr const char DeleteMd5SumStatement[]               = "DELETE FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char DeleteRefreshStatement[]              = "DELETE FROM refresh WHERE UUID = :UUID";
static constexpr const char DeleteErrorEntryStatement[]           = "DELETE FROM error_entry WHERE UUID = :UUID AND progress_num = :progress_num";

static constexpr const char GetDBVersionStatement[]               = "SELECT * FROM db_version";
static constexpr const char GetWorkEntriesStatement[]             = "SELECT * FROM work_entry";
static constexpr const char GetMd5sumsStatement[]                 = "SELECT * FROM md5_sum";
static constexpr const char GetErrorEntriesStatement[]            = "SELECT * FROM error_entry";

static constexpr const char DoesMinRefreshExistStatement[]        = "SELECT CASE WHEN EXISTS(SELECT 1 FROM refresh) THEN 1 ELSE 0 END";
static constexpr const char GetWorkEntryUUIDFromUrlStatement[]    = "SELECT UUID FROM work_entry WHERE url = :url";
static constexpr const char GetWorkEntryUrlFromUUIDStatement[]    = "SELECT url, last_url FROM work_entry WHERE UUID = :UUID";
static constexpr const char GetMd5SumFromMd5SumStatement[]        = "SELECT * FROM md5_sum WHERE UUID = :UUID AND md5_sum = :md5_sum";
static constexpr const char GetMd5SumFromUUIDAndIndexStatement[]  = "SELECT * FROM md5_sum WHERE UUID = :UUID AND index_num = :index_num";
static constexpr const char GetMd5SumsFromUUIDStatement[]         = "SELECT * FROM md5_sum WHERE UUID = :UUID ORDER BY index_num ASC";
static constexpr const char GetRefreshFromMinDateStatement[]      = "SELECT * FROM refresh WHERE refresh_date=(SELECT MIN(refresh_date) FROM refresh)";

typedef enum {
    CREATE_DB_VERSION_STATEMENT,
    CREATE_USER_STATEMENT,
    CREATE_MEDIA_TYPE_STATEMENT,
    CREATE_MEDIA_SUBTYPE_STATEMENT,
    CREATE_SOURCE_STATEMENT,
    CREATE_WORK_ENTRY_STATEMENT,
    CREATE_MD5_SUM_STATEMENT,
    CREATE_REFRESH_STATEMENT,
    CREATE_ERROR_ENTRY_STATEMENT,

    READ_WORK_ENTRY_STATEMENT,
    READ_WORK_ENTRY_URL_STATEMENT,
    READ_WORK_ENTRY_UUID_STATEMENT,
    READ_MD5_SUM_BY_INDEX_NUM_STATEMENT,
    READ_MD5_SUM_BY_SEC_ID_STATEMENT,
    READ_MD5_SUM_BY_SEQ_NUM_STATEMENT,
    READ_REFRESH_STATEMENT,
    READ_ERROR_ENTRY_STATEMENT,

    UPDATE_WORK_ENTRY_STATEMENT,
    UPDATE_MD5_SUM_STATEMENT,

    DELETE_WORK_ENTRY_STATEMENT,
    DELETE_MD5_SUM_STATEMENT,
    DELETE_REFRESH_STATEMENT,
    DELETE_ERROR_ENTRY_STATEMENT,

    GET_DB_VERSION_STATEMENT,
    GET_WORK_ENTRIES_STATEMENT,
    GET_CHECKSUMS_STATEMENT,
    GET_ERROR_ENTRIES_STATEMENT,

    DOES_MIN_REFRESH_EXIST_STATEMENT,
    GET_WORK_ENTRY_UUID_FROM_URL_STATEMENT,
    GET_WORK_ENTRY_URL_FROM_UUID_STATEMENT,
    GET_MD5_SUM_FROM_MD5_SUM_STATEMENT,
    GET_MD5_SUM_FROM_UUID_AND_INDEX_STATEMENT,
    GET_MD5_SUMS_FROM_UUID_STATEMENT,
    GET_REFRESH_FROM_MIN_DATE_STATEMENT,

    _NUM_PREPARED_STATEMENTS
} prepared_statement_id_t;

SQLiteDB::SQLiteDB(const std::string &database_url, const std::string &db_version) :
    database_conn_(),
    prepared_statements_(),
    logger_name_("db"),
    initialized_(false)
{
    std::string target_url = database_url;
    if (target_url.empty())
    {
        target_url = DefaultDBPath;
        BlackLibraryCommon::LogDebug(logger_name_, "Empty database url given, using default: {}", target_url);
    }

    bool first_time_setup = false;
    if (!BlackLibraryCommon::PathExists(target_url))
    {
        BlackLibraryCommon::LogDebug(logger_name_, "{} does not exist, first time setup enabled", target_url);
        first_time_setup = true;
    }

    int res = sqlite3_open(target_url.c_str(), &database_conn_);
    
    if (res != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to open db at: {} - {}", target_url, sqlite3_errmsg(database_conn_));
        return;
    }

    BlackLibraryCommon::LogInfo(logger_name_, "Open database at: {}", target_url);

    if (first_time_setup)
    {
        if (GenerateTables())
        {
            BlackLibraryCommon::LogError(logger_name_, "Failed to generate database tables");
            return;
        }
    }

    if (PrepareStatements())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to setup prepare statements");
        return;
    }

    if (first_time_setup)
    {
        if (SetupDefaultLibraryUsers())
        {
            BlackLibraryCommon::LogError(logger_name_, "Failed to setup default black library users");
            return;
        }
        if (SetupDefaultTypeTables())
        {
            BlackLibraryCommon::LogError(logger_name_, "Failed to setup default type tables");
            return;
        }
        if (SetupVersionTable(db_version))
        {
            BlackLibraryCommon::LogError(logger_name_, "Failed to setup version table");
            return;
        }
    }

    if (CheckDBVersion(db_version))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed db version check");
        return;
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

std::vector<DBEntry> SQLiteDB::ListEntries() const
{
    BlackLibraryCommon::LogDebug(logger_name_, "List entries");

    std::vector<DBEntry> entries;

    if (CheckInitialized())
        return entries;

    if (BeginTransaction())
        return entries;

    sqlite3_stmt *stmt = prepared_statements_[GET_WORK_ENTRIES_STATEMENT];

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DBEntry entry;

        entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::uuid))));
        entry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::title))));
        entry.author = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::author))));
        entry.nickname = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::nickname))));
        entry.source = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::source))));
        entry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::url))));
        entry.last_url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::last_url))));
        entry.series = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::series))));
        entry.series_length = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::series_length));
        entry.version = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::version));
        entry.media_path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::media_path))));
        entry.birth_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::birth_date));
        entry.check_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::check_date));
        entry.update_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::update_date));
        entry.user_contributed = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::user_contributed));
        entry.processing = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::processing));

        entries.emplace_back(entry);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return entries;

    return entries;
}

std::vector<BlackLibraryCommon::Md5Sum> SQLiteDB::ListChecksums() const
{
    BlackLibraryCommon::LogDebug(logger_name_, "List checksums");

    std::vector<BlackLibraryCommon::Md5Sum> md5_sums;

    if (CheckInitialized())
        return md5_sums;

    if (BeginTransaction())
        return md5_sums;

    sqlite3_stmt *stmt = prepared_statements_[GET_CHECKSUMS_STATEMENT];

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        BlackLibraryCommon::Md5Sum md5_sum;

        md5_sum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
        md5_sum.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
        md5_sum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
        md5_sum.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
        md5_sum.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
        md5_sum.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
        md5_sum.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

        md5_sums.emplace_back(md5_sum);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return md5_sums;

    return md5_sums;
}

std::vector<DBErrorEntry> SQLiteDB::ListErrorEntries() const
{
    BlackLibraryCommon::LogDebug(logger_name_, "List error entries");

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

        entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBErrorEntryColumnID::uuid))));
        entry.progress_num = sqlite3_column_int(stmt, static_cast<unsigned int>(DBErrorEntryColumnID::progress_num));

        entries.emplace_back(entry);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return entries;

    return entries;
}

int SQLiteDB::CreateDBVersion(const std::string &db_version) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Create db version: {} ", db_version);

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_DB_VERSION_STATEMENT];

    if (BindText(stmt, "db_version", db_version))
        return -1;

    // run statement
    int ret = SQLITE_OK;

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Create db_version {} failed: {}", db_version, sqlite3_errmsg(database_conn_));
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateUser(const DBUser &user) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Create user: {} with UID: {}", user.name, user.uid);

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
        BlackLibraryCommon::LogError(logger_name_, "Create user: {} with UID: {} failed: {}", user.name, user.uid, sqlite3_errmsg(database_conn_));
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateMediaType(const std::string &media_type_name) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Create media type: {}", media_type_name);

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
        BlackLibraryCommon::LogError(logger_name_, "Create media type: {} failed: {}", media_type_name, sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Create subtype: {} media: {}", media_subtype_name, media_type_name);

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
        BlackLibraryCommon::LogError(logger_name_, "Create subtype: {} - media: {} failed: {}", media_subtype_name, media_type_name, sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Create source: {}", source.name);

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
        BlackLibraryCommon::LogError(logger_name_, "Create source: {} failed: {}", source.name, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateEntry(const DBEntry &entry) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Create entry with UUID: {}", entry.uuid);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[CREATE_WORK_ENTRY_STATEMENT];

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
    if (BindInt(stmt, "processing", entry.processing))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Create entry failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

DBEntry SQLiteDB::ReadEntry(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Read entry with UUID: {}", uuid);

    DBEntry entry;

    if (CheckInitialized())
        return entry;

    if (BeginTransaction())
        return entry;

    sqlite3_stmt *stmt = prepared_statements_[READ_WORK_ENTRY_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return entry;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError(logger_name_, "Read entry failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return entry;
    }

    entry.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::uuid))));
    entry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::title))));
    entry.author = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::author))));
    entry.nickname = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::nickname))));
    entry.source = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::source))));
    entry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::url))));
    entry.last_url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::last_url))));
    entry.series = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::series))));
    entry.series_length = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::series_length));
    entry.version = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::version));
    entry.media_path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBEntryColumnID::media_path))));
    entry.birth_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::birth_date));
    entry.check_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::check_date));
    entry.update_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::update_date));
    entry.user_contributed = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::user_contributed));
    entry.processing = sqlite3_column_int(stmt, static_cast<unsigned int>(DBEntryColumnID::processing));

    ResetStatement(stmt);

    if (EndTransaction())
        return entry;

    return entry;
}

int SQLiteDB::UpdateEntry(const DBEntry &entry) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Update entry with UUID: {}", entry.uuid);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[UPDATE_WORK_ENTRY_STATEMENT];

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
    if (BindInt(stmt, "processing", entry.processing))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Update entry failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::DeleteEntry(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Delete entry with UUID: {}", uuid);

    if (CheckInitialized())
        return -1;

    if (BeginTransaction())
        return -1;

    sqlite3_stmt *stmt = prepared_statements_[DELETE_WORK_ENTRY_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Delete entry failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

int SQLiteDB::CreateMd5Sum(const BlackLibraryCommon::Md5Sum &md5) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Create MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);

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
    if (BindInt(stmt, "date", md5.date))
        return -1;
    if (BindText(stmt, "sec_id", md5.sec_id))
        return -1;
    if (BindInt(stmt, "seq_num", md5.seq_num))
        return -1;
    if (BindInt(stmt, "version_num", md5.version_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Create MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

BlackLibraryCommon::Md5Sum SQLiteDB::ReadMd5SumByIndexNum(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Read MD5 checksum with UUID: {} index_num: {}", uuid, index_num);

    BlackLibraryCommon::Md5Sum checksum;

    if (CheckInitialized())
        return checksum;

    if (BeginTransaction())
        return checksum;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_INDEX_NUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return checksum;
    if (BindInt(stmt, "index_num", index_num))
        return checksum;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError(logger_name_, "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return checksum;
    }

    checksum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
    checksum.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
    checksum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
    checksum.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
    checksum.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
    checksum.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
    checksum.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

    ResetStatement(stmt);

    if (EndTransaction())
        return checksum;

    return checksum;
}

BlackLibraryCommon::Md5Sum SQLiteDB::ReadMd5SumBySecId(const std::string &uuid, const std::string &sec_id) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Read MD5 checksum with UUID: {} sec_id: {}", uuid, sec_id);

    BlackLibraryCommon::Md5Sum checksum;

    if (CheckInitialized())
        return checksum;

    if (BeginTransaction())
        return checksum;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_SEC_ID_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return checksum;
    if (BindText(stmt, "sec_id", sec_id))
        return checksum;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError(logger_name_, "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return checksum;
    }

    checksum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
    checksum.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
    checksum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
    checksum.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
    checksum.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
    checksum.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
    checksum.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

    ResetStatement(stmt);

    if (EndTransaction())
        return checksum;

    return checksum;
}

BlackLibraryCommon::Md5Sum SQLiteDB::ReadMd5SumBySeqNum(const std::string &uuid, const size_t &seq_num) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Read MD5 checksum with UUID: {} seq_num: {}", uuid, seq_num);

    BlackLibraryCommon::Md5Sum checksum;

    if (CheckInitialized())
        return checksum;

    if (BeginTransaction())
        return checksum;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_SEQ_NUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return checksum;
    if (BindInt(stmt, "seq_num", seq_num))
        return checksum;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError(logger_name_, "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return checksum;
    }

    checksum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
    checksum.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
    checksum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
    checksum.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
    checksum.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
    checksum.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
    checksum.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

    ResetStatement(stmt);

    if (EndTransaction())
        return checksum;

    return checksum;
}

int SQLiteDB::UpdateMd5Sum(const BlackLibraryCommon::Md5Sum &md5) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Update MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);

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
    if (BindInt(stmt, "date", md5.date))
        return -1;
    if (BindText(stmt, "sec_id", md5.sec_id))
        return -1;
    if (BindInt(stmt, "seq_num", md5.seq_num))
        return -1;
    if (BindInt(stmt, "version_num", md5.version_num))
        return -1;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        BlackLibraryCommon::LogError(logger_name_, "Update MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Delete MD5 checksum with UUID: {} index_num: {}", uuid, index_num);

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
        BlackLibraryCommon::LogError(logger_name_, "Delete MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Create refresh with UUID: {} refresh_date: {}", refresh.uuid, refresh.refresh_date);

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
        BlackLibraryCommon::LogError(logger_name_, "Create refresh checksum failed: {}", sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Read refresh with UUID: {}", uuid);

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
        BlackLibraryCommon::LogError(logger_name_, "Read refresh failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return refresh;
    }

    refresh.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBRefreshColumnID::uuid))));
    refresh.refresh_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBRefreshColumnID::refresh_date));

    ResetStatement(stmt);

    if (EndTransaction())
        return refresh;

    return refresh;
}

int SQLiteDB::DeleteRefresh(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Delete refresh with UUID: {}", uuid);

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
        BlackLibraryCommon::LogError(logger_name_, "Delete refresh failed: {}", sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Create error entry for UUID: {}", entry.uuid);

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
        BlackLibraryCommon::LogError(logger_name_, "Create error entry for UUID: {} failed: {}", entry.uuid, sqlite3_errmsg(database_conn_));
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
    BlackLibraryCommon::LogDebug(logger_name_, "Delete error entry for UUID: {} and progress number: {}", uuid, progress_num);

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
        BlackLibraryCommon::LogError(logger_name_, "Delete error entry for UUID: {} and progress number: {} failed: {}", uuid, progress_num, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return -1;

    return 0;
}

DBBoolResult SQLiteDB::DoesEntryUrlExist(const std::string &url) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Check entries for url: {}", url);

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

    sqlite3_stmt *stmt = prepared_statements_[READ_WORK_ENTRY_URL_STATEMENT];

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
        BlackLibraryCommon::LogDebug(logger_name_, "Entry url: {} does not exist", url);
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

DBBoolResult SQLiteDB::DoesEntryUUIDExist(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Check entries for UUID: {}", uuid);

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

    sqlite3_stmt *stmt = prepared_statements_[READ_WORK_ENTRY_UUID_STATEMENT];

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
        BlackLibraryCommon::LogDebug(logger_name_, "Entry UUID: {} does not exist", uuid);
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

DBBoolResult SQLiteDB::DoesMd5SumExistByIndexNum(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Check MD5 checksum for UUID: {}, index_num: {}", uuid, index_num);

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

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_INDEX_NUM_STATEMENT];

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
        BlackLibraryCommon::LogDebug(logger_name_, "MD5 checksum UUID: {} index_num: {} does not exist", uuid, index_num);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }

    check.result = true;

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesMd5SumExistBySecId(const std::string &uuid, const std::string &sec_id) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Check MD5 checksum for UUID: {}, sec_id: {}", uuid, sec_id);

    DBBoolResult check;

    if (uuid.empty() || sec_id.empty())
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

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_SEC_ID_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }
    if (BindText(stmt, "sec_id", sec_id))
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
        BlackLibraryCommon::LogDebug(logger_name_, "MD5 checksum UUID: {} sec_id: {} does not exist", uuid, sec_id);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }

    check.result = true;

    ResetStatement(stmt);

    if (EndTransaction())
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }

    return check;
}

DBBoolResult SQLiteDB::DoesMd5SumExistBySeqNum(const std::string &uuid, const size_t &seq_num) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Check MD5 checksum for UUID: {}, seq_num: {}", uuid, seq_num);

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

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_SEQ_NUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
    {
        check.error = sqlite3_errcode(database_conn_);
        return check;
    }
    if (BindInt(stmt, "seq_num", seq_num))
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
        BlackLibraryCommon::LogDebug(logger_name_, "MD5 checksum UUID: {} seq_num: {} does not exist", uuid, seq_num);
        check.result = false;
        ResetStatement(stmt);
        EndTransaction();
        return check;
    }

    check.result = true;

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
    BlackLibraryCommon::LogDebug(logger_name_, "Check refresh for UUID: {}", uuid);

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
        BlackLibraryCommon::LogDebug(logger_name_, "refresh UUID: {} does not exist", uuid);
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
    BlackLibraryCommon::LogDebug(logger_name_, "Check error entries for UUID: {}, progress_num: {}", uuid, progress_num);

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
        BlackLibraryCommon::LogDebug(logger_name_, "UUID: {} progress_num: {} does not exist", uuid, progress_num);
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
    BlackLibraryCommon::LogDebug(logger_name_, "Check refresh for min date");

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
        BlackLibraryCommon::LogDebug(logger_name_, "refresh does not exist");
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

DBStringResult SQLiteDB::GetDBVersion() const
{
    DBStringResult res;

    if (BeginTransaction())
        return res;

    sqlite3_stmt *stmt = prepared_statements_[GET_DB_VERSION_STATEMENT];

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogError(logger_name_, "Get DB version failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return res;
    }

    res.result = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

    ResetStatement(stmt);

    if (EndTransaction())
        return res;

    res.error = false;

    return res;
}

DBStringResult SQLiteDB::GetEntryUUIDFromUrl(const std::string &url) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get UUID from url: {}", url);

    DBStringResult res;

    if (CheckInitialized())
        return res;

    if (BeginTransaction())
        return res;

    sqlite3_stmt *stmt = prepared_statements_[GET_WORK_ENTRY_UUID_FROM_URL_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "url", url))
        return res;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug(logger_name_, "Url: {} does not exist", url);
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

DBStringResult SQLiteDB::GetEntryUrlFromUUID(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get url from UUID: {}", uuid);

    DBStringResult res;

    if (CheckInitialized())
        return res;

    if (BeginTransaction())
        return res;

    sqlite3_stmt *stmt = prepared_statements_[GET_WORK_ENTRY_URL_FROM_UUID_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return res;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug(logger_name_, "UUID: {} does not exist", uuid);
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

BlackLibraryCommon::Md5Sum SQLiteDB::GetMd5SumFromMd5Sum(const std::string &md5_sum, const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get md5 from: {}", md5_sum);

    BlackLibraryCommon::Md5Sum md5;

    if (CheckInitialized())
        return md5;

    if (BeginTransaction())
        return md5;

    sqlite3_stmt *stmt = prepared_statements_[GET_MD5_SUM_FROM_MD5_SUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "md5_sum", md5_sum))
        return md5;
    if (BindText(stmt, "UUID", uuid))
        return md5;

    LogTraceStatement(stmt);

    // run statement
    int ret = SQLITE_OK;
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW)
    {
        BlackLibraryCommon::LogDebug(logger_name_, "Read MD5 checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return md5;
    }

    md5.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
    md5.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
    md5.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
    md5.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
    md5.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
    md5.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
    md5.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

    ResetStatement(stmt);

    if (EndTransaction())
        return md5;

    return md5;
}

std::vector<BlackLibraryCommon::Md5Sum> SQLiteDB::GetMd5SumsFromUUID(const std::string &uuid) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get md5 sums from: {}", uuid);

    std::vector<BlackLibraryCommon::Md5Sum> md5_sums;

    if (CheckInitialized())
        return md5_sums;

    if (BeginTransaction())
        return md5_sums;

    sqlite3_stmt *stmt = prepared_statements_[GET_MD5_SUMS_FROM_UUID_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return md5_sums;

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        BlackLibraryCommon::Md5Sum md5_sum;

        md5_sum.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::uuid))));
        md5_sum.index_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::index_num));
        md5_sum.md5_sum = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::md5_sum))));
        md5_sum.date = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::date));
        md5_sum.sec_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::sec_id))));
        md5_sum.seq_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::seq_num));
        md5_sum.version_num = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

        md5_sums.emplace_back(md5_sum);
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return md5_sums;

    return md5_sums;
}

uint16_t SQLiteDB::GetVersionFromMd5(const std::string &uuid, size_t index_num) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get version from MD5");

    uint16_t version_num = 0;

    if (CheckInitialized())
        return version_num;

    if (BeginTransaction())
        return version_num;

    sqlite3_stmt *stmt = prepared_statements_[READ_MD5_SUM_BY_INDEX_NUM_STATEMENT];

    // bind statement variables
    if (BindText(stmt, "UUID", uuid))
        return version_num;
    if (BindInt(stmt, "index_num", index_num))
        return version_num;

    LogTraceStatement(stmt);

    // run statement in loop until done
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        auto version_num_check = sqlite3_column_int(stmt, static_cast<unsigned int>(BlackLibraryCommon::DBMd5SumColumnID::version_num));

        if (version_num_check > version_num)
            version_num = version_num_check;
    }

    ResetStatement(stmt);

    if (EndTransaction())
        return version_num;

    return version_num;
}

DBRefresh SQLiteDB::GetRefreshFromMinDate() const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Get refresh from next date");

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
        BlackLibraryCommon::LogError(logger_name_, "Read refresh checksum failed: {}", sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return refresh;
    }

    refresh.uuid = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<unsigned int>(DBRefreshColumnID::uuid))));
    refresh.refresh_date = sqlite3_column_int(stmt, static_cast<unsigned int>(DBRefreshColumnID::refresh_date));

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
    BlackLibraryCommon::LogDebug(logger_name_, "Setting up tables");

    int res = 0;

    res += GenerateTable(CreateDBVersionTable);
    res += GenerateTable(CreateUserTable);
    res += GenerateTable(CreateMediaTypeTable);
    res += GenerateTable(CreateMediaSubtypeTable);
    res += GenerateTable(CreateBookGenreTable);
    res += GenerateTable(CreateDocumentTagTable);
    res += GenerateTable(CreateSourceTable);
    res += GenerateTable(CreateWorkEntryTable);
    res += GenerateTable(CreateMd5SumTable);
    res += GenerateTable(CreateRefreshTable);
    res += GenerateTable(CreateErrorEntryTable);

    return res;
}

int SQLiteDB::SetupDefaultTypeTables()
{
    BlackLibraryCommon::LogDebug(logger_name_, "Setting up default type tables");

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
    DBSource wp_source;
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

    wp_source.name = BlackLibraryCommon::WP::source_name;
    wp_source.media_type = DBEntryMediaType::Document;
    wp_source.subtype = DBEntryMediaSubtype::BLOG;

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
    if (CreateSource(wp_source))
        return -1;
    if (CreateSource(yt_source))
        return -1;

    return 0;
}

int SQLiteDB::SetupVersionTable(const std::string &db_version)
{
    BlackLibraryCommon::LogDebug(logger_name_, "Setting up version table");

    if (CreateDBVersion(db_version))
        return -1;

    return 0;
}

int SQLiteDB::PrepareStatements()
{
    if (!database_conn_)
        return -1;

    int res = 0;

    res += PrepareStatement(CreateDBVersionStatement, CREATE_DB_VERSION_STATEMENT);
    res += PrepareStatement(CreateUserStatement, CREATE_USER_STATEMENT);
    res += PrepareStatement(CreateMediaTypeStatement, CREATE_MEDIA_TYPE_STATEMENT);
    res += PrepareStatement(CreateMediaSubtypeStatement, CREATE_MEDIA_SUBTYPE_STATEMENT);
    res += PrepareStatement(CreateSourceStatement, CREATE_SOURCE_STATEMENT);
    res += PrepareStatement(CreateWorkEntryStatement, CREATE_WORK_ENTRY_STATEMENT);
    res += PrepareStatement(CreateMd5SumStatement, CREATE_MD5_SUM_STATEMENT);
    res += PrepareStatement(CreateRefreshStatement, CREATE_REFRESH_STATEMENT);
    res += PrepareStatement(CreateErrorEntryStatement, CREATE_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(ReadWorkEntryStatement, READ_WORK_ENTRY_STATEMENT);
    res += PrepareStatement(ReadWorkEntryUrlStatement, READ_WORK_ENTRY_URL_STATEMENT);
    res += PrepareStatement(ReadWorkEntryUUIDStatement, READ_WORK_ENTRY_UUID_STATEMENT);
    res += PrepareStatement(ReadMd5SumByIndexNumStatement, READ_MD5_SUM_BY_INDEX_NUM_STATEMENT);
    res += PrepareStatement(ReadMd5SumBySecIdStatement, READ_MD5_SUM_BY_SEC_ID_STATEMENT);
    res += PrepareStatement(ReadMd5SumBySeqNumStatement, READ_MD5_SUM_BY_SEQ_NUM_STATEMENT);
    res += PrepareStatement(ReadRefreshStatement, READ_REFRESH_STATEMENT);
    res += PrepareStatement(ReadErrorEntryStatement, READ_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(UpdateWorkEntryStatement, UPDATE_WORK_ENTRY_STATEMENT);
    res += PrepareStatement(UpdateMd5SumStatement, UPDATE_MD5_SUM_STATEMENT);

    res += PrepareStatement(DeleteWorkEntryStatement, DELETE_WORK_ENTRY_STATEMENT);
    res += PrepareStatement(DeleteMd5SumStatement, DELETE_MD5_SUM_STATEMENT);
    res += PrepareStatement(DeleteRefreshStatement, DELETE_REFRESH_STATEMENT);
    res += PrepareStatement(DeleteErrorEntryStatement, DELETE_ERROR_ENTRY_STATEMENT);

    res += PrepareStatement(GetDBVersionStatement, GET_DB_VERSION_STATEMENT);
    res += PrepareStatement(GetWorkEntriesStatement, GET_WORK_ENTRIES_STATEMENT);
    res += PrepareStatement(GetMd5sumsStatement, GET_CHECKSUMS_STATEMENT);
    res += PrepareStatement(GetErrorEntriesStatement, GET_ERROR_ENTRIES_STATEMENT);

    res += PrepareStatement(DoesMinRefreshExistStatement, DOES_MIN_REFRESH_EXIST_STATEMENT);
    res += PrepareStatement(GetWorkEntryUUIDFromUrlStatement, GET_WORK_ENTRY_UUID_FROM_URL_STATEMENT);
    res += PrepareStatement(GetWorkEntryUrlFromUUIDStatement, GET_WORK_ENTRY_URL_FROM_UUID_STATEMENT);
    res += PrepareStatement(GetMd5SumFromMd5SumStatement, GET_MD5_SUM_FROM_MD5_SUM_STATEMENT);
    res += PrepareStatement(GetMd5SumFromUUIDAndIndexStatement, GET_MD5_SUM_FROM_UUID_AND_INDEX_STATEMENT);
    res += PrepareStatement(GetMd5SumsFromUUIDStatement, GET_MD5_SUMS_FROM_UUID_STATEMENT);
    res += PrepareStatement(GetRefreshFromMinDateStatement, GET_REFRESH_FROM_MIN_DATE_STATEMENT);

    return res;
}

int SQLiteDB::SetupDefaultLibraryUsers()
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
    BlackLibraryCommon::LogTrace(logger_name_, "Begin transaction");
    int ret = sqlite3_exec(database_conn_, "BEGIN TRANSACTION", 0, 0, &error_msg);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "Begin transaction failed: {} - {}", error_msg, sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::CheckDBVersion(const std::string &db_version) const
{
    BlackLibraryCommon::LogDebug(logger_name_, "Checking db version");

    auto res = GetDBVersion();

    // if version mismatch, fail
    if (db_version != res.result)
    {
        return -1;
    }

    return 0;
}

int SQLiteDB::CheckInitialized() const
{
    if (!initialized_)
    {
        BlackLibraryCommon::LogError(logger_name_, "Database not initialized");
        return -1;
    }

    return 0;
}

int SQLiteDB::EndTransaction() const
{
    char *error_msg = 0;
    BlackLibraryCommon::LogTrace(logger_name_, "End transaction");
    int ret = sqlite3_exec(database_conn_, "END TRANSACTION", 0, 0, &error_msg);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "End transaction  failed: {} - {}", error_msg, sqlite3_errmsg(database_conn_));
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
        BlackLibraryCommon::LogError(logger_name_, "Generate table failed: {}", sqlite3_errmsg(database_conn_));
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
        BlackLibraryCommon::LogError(logger_name_, "Prepare failed: {} - statement: {}", sqlite3_errmsg(database_conn_), statement);
        return -1;
    }

    return 0;
}

int SQLiteDB::ResetStatement(sqlite3_stmt* stmt) const
{
    int ret = sqlite3_reset(stmt);
    BlackLibraryCommon::LogTrace(logger_name_, "Reset statement");
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "Reset statement failed: {}", sqlite3_errmsg(database_conn_));
        return -1;
    }

    return 0;
}

int SQLiteDB::BindInt(sqlite3_stmt* stmt, const std::string &parameter_name, const int &bind_int) const
{
    BlackLibraryCommon::LogTrace(logger_name_, "BindInt parameter:{} with {}", parameter_name, bind_int);
    const std::string parameter_index_name = ":" + parameter_name;
    int ret = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, parameter_index_name.c_str()), bind_int);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "Bind of {}: {} failed: {}", parameter_name, bind_int, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    return 0;
}

int SQLiteDB::BindText(sqlite3_stmt* stmt, const std::string &parameter_name, const std::string &bind_text) const
{
    BlackLibraryCommon::LogTrace(logger_name_, "BindText parameter:{} with {}", parameter_name, bind_text);
    const std::string parameter_index_name = ":" + parameter_name;
    int index = sqlite3_bind_parameter_index(stmt, parameter_index_name.c_str());
    int ret = sqlite3_bind_text(stmt, index, bind_text.c_str(), bind_text.length(), SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        BlackLibraryCommon::LogError(logger_name_, "Bind of {}: {} failed: {}", parameter_name, bind_text, sqlite3_errmsg(database_conn_));
        ResetStatement(stmt);
        EndTransaction();
        return -1;
    }

    return 0;
}

int SQLiteDB::LogDebugStatement(sqlite3_stmt* stmt) const
{
    char *debug_sql = sqlite3_expanded_sql(stmt);
    BlackLibraryCommon::LogDebug(logger_name_, "{}", std::string(debug_sql));
    sqlite3_free(debug_sql);

    return 0;
}

int SQLiteDB::LogTraceStatement(sqlite3_stmt* stmt) const
{
    char *trace_sql = sqlite3_expanded_sql(stmt);
    BlackLibraryCommon::LogTrace(logger_name_, "{}", std::string(trace_sql));
    sqlite3_free(trace_sql);

    return 0;
}

} // namespace db
} // namespace core
} // namespace black_library
