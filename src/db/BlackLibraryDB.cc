/**
 * BlackLibraryDB.cc
 */

#include <iostream>

#include <LogOperations.h>

#include <SQLiteDB.h>

#include <BlackLibraryDB.h>

namespace black_library {

namespace core {

namespace db {

namespace BlackLibraryCommon = black_library::core::common;

BlackLibraryDB::BlackLibraryDB(const njson &config) :
    database_connection_interface_(nullptr),
    mutex_()
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string database_url = DefaultDBPath;
    if (nconfig.contains("db_path"))
    {
        database_url = nconfig["db_path"];
    }

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    bool logger_level = BlackLibraryCommon::DefaultLogLevel;
    if (nconfig.contains("db_debug_log"))
    {
        logger_level = nconfig["db_debug_log"];
    }

    BlackLibraryCommon::InitRotatingLogger("db", logger_path, logger_level);

    database_connection_interface_ = std::make_unique<SQLiteDB>(database_url);
}

BlackLibraryDB::~BlackLibraryDB()
{
}

std::vector<DBEntry> BlackLibraryDB::GetWorkEntryList()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    auto entry_list = database_connection_interface_->ListEntries();

    return entry_list;
}

std::vector<DBMd5Sum> BlackLibraryDB::GetChecksumList()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    auto checksum_list = database_connection_interface_->ListChecksums();

    return checksum_list;
}

std::vector<DBErrorEntry> BlackLibraryDB::GetErrorEntryList()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    auto entry_list = database_connection_interface_->ListErrorEntries();

    return entry_list;
}

int BlackLibraryDB::CreateWorkEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->CreateEntry(entry))
    {
        BlackLibraryCommon::LogError("db", "Failed to create black entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

DBEntry BlackLibraryDB::ReadWorkEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBEntry entry;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read black entry with empty UUID");
        return entry;
    }
    entry = database_connection_interface_->ReadEntry(uuid);
    if (entry.uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read black entry with UUID: {}", uuid);
        return entry;
    }

    return entry;
}

int BlackLibraryDB::UpdateWorkEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->UpdateEntry(entry))
    {
        BlackLibraryCommon::LogError("db", "Failed to update black entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteWorkEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete black entry with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteEntry(uuid))
    {
        BlackLibraryCommon::LogError("db", "Failed to delete black entry with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateMd5Sum(const DBMd5Sum &md5)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (md5.uuid.empty() || database_connection_interface_->CreateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError("db", "Failed to create MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);
        return -1;
    }

    return 0;
}

DBMd5Sum BlackLibraryDB::ReadMd5Sum(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBMd5Sum md5;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read MD5 checksum with empty UUID");
        return md5;
    }
    md5 = database_connection_interface_->ReadMd5Sum(uuid, index_num);
    if (md5.uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read MD5 checksum with UUID: {} index_num: {}", uuid, index_num);
        return md5;
    }

    return md5;
}

uint16_t BlackLibraryDB::GetVersionFromMd5(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    uint16_t version_num = 0;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to get version from MD5 checksum with empty UUID");
        return version_num;
    }
    version_num = database_connection_interface_->GetVersionFromMd5(uuid, index_num);

    return version_num;
}

int BlackLibraryDB::UpdateMd5Sum(const DBMd5Sum &md5)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (md5.uuid.empty() || database_connection_interface_->UpdateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError("db", "Failed to update MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteMd5Sum(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete MD5 checksum with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteMd5Sum(uuid, index_num))
    {
        BlackLibraryCommon::LogError("db", "Failed to delete MD5 checksum with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateRefresh(const DBRefresh &refresh)
{
    if (refresh.uuid.empty() || database_connection_interface_->CreateRefresh(refresh))
    {
        BlackLibraryCommon::LogError("db", "Failed to create refresh with UUID: {} refresh_date: {}", refresh.uuid, refresh.refresh_date);
        return -1;
    }

    return 0;
}

DBRefresh BlackLibraryDB::ReadRefresh(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBRefresh refresh;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read refresh with empty UUID");
        return refresh;
    }
    refresh = database_connection_interface_->ReadRefresh(uuid);
    if (refresh.uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read refresh with UUID: {}", uuid);
        return refresh;
    }

    return refresh;
}

int BlackLibraryDB::DeleteRefresh(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete refresh with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteRefresh(uuid))
    {
        BlackLibraryCommon::LogError("db", "Failed to delete refresh with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateErrorEntry(const DBErrorEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->CreateErrorEntry(entry))
    {
        BlackLibraryCommon::LogError("db", "Failed to create error entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteErrorEntry(const std::string &uuid, size_t progress_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete error entry with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteErrorEntry(uuid, progress_num))
    {
        BlackLibraryCommon::LogError("db", "Failed to delete error entry with UUID: {} and progress number: {}", uuid, progress_num);
        return -1;
    }

    return 0;
}

bool BlackLibraryDB::DoesWorkEntryUrlExist(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUrlExist(url);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesWorkEntryUUIDExist(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUUIDExist(uuid);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesMd5SumExist(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesMd5SumExist(uuid, index_num);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesRefreshExist(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesRefreshExist(uuid);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesMinRefreshExist()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesMinRefreshExist();
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesErrorEntryExist(const std::string &uuid, size_t progress_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesErrorEntryExist(uuid, progress_num);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

DBStringResult BlackLibraryDB::GetWorkEntryUUIDFromUrl(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBStringResult res = database_connection_interface_->GetEntryUUIDFromUrl(url);
    if (res.error)
        BlackLibraryCommon::LogError("db", "Failed to get black UUID from url: {}", url);

    return res;
}

DBStringResult BlackLibraryDB::GetWorkEntryUrlFromUUID(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetEntryUrlFromUUID(uuid);
}

DBRefresh BlackLibraryDB::GetRefreshFromMinDate()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetRefreshFromMinDate();
}

bool BlackLibraryDB::IsReady()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->IsReady();
}

} // namespace db
} // namespace core
} // namespace black_library
