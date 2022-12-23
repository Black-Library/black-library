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

std::vector<DBEntry> BlackLibraryDB::GetStagingEntryList()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    auto entry_list = database_connection_interface_->ListEntries(STAGING_ENTRY);

    return entry_list;
}

std::vector<DBEntry> BlackLibraryDB::GetBlackEntryList()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    auto entry_list = database_connection_interface_->ListEntries(BLACK_ENTRY);

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

int BlackLibraryDB::CreateStagingEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->CreateEntry(entry, STAGING_ENTRY))
    {
        BlackLibraryCommon::LogError("db", "Failed to create staging entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

DBEntry BlackLibraryDB::ReadStagingEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBEntry entry;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read staging entry with empty UUID");
        return entry;
    }
    entry = database_connection_interface_->ReadEntry(uuid, STAGING_ENTRY);
    if (entry.uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read staging entry with UUID: {}", uuid);
        return entry;
    }

    return entry;
}

int BlackLibraryDB::UpdateStagingEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->UpdateEntry(entry, STAGING_ENTRY))
    {
        BlackLibraryCommon::LogError("db", "Failed to update staging entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteStagingEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete staging entry with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteEntry(uuid, STAGING_ENTRY))
    {
        BlackLibraryCommon::LogError("db", "Failed to delete staging entry with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateBlackEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->CreateEntry(entry, BLACK_ENTRY))
    {
        BlackLibraryCommon::LogError("db", "Failed to create black entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

DBEntry BlackLibraryDB::ReadBlackEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBEntry entry;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read black entry with empty UUID");
        return entry;
    }
    entry = database_connection_interface_->ReadEntry(uuid, BLACK_ENTRY);
    if (entry.uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to read black entry with UUID: {}", uuid);
        return entry;
    }

    return entry;
}

int BlackLibraryDB::UpdateBlackEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->UpdateEntry(entry, BLACK_ENTRY))
    {
        BlackLibraryCommon::LogError("db", "Failed to update black entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteBlackEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError("db", "Failed to delete black entry with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteEntry(uuid, BLACK_ENTRY))
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


bool BlackLibraryDB::DoesStagingEntryUrlExist(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUrlExist(url, STAGING_ENTRY);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesBlackEntryUrlExist(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUrlExist(url, BLACK_ENTRY);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesStagingEntryUUIDExist(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUUIDExist(uuid, STAGING_ENTRY);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError("db", "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesBlackEntryUUIDExist(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesEntryUUIDExist(uuid, BLACK_ENTRY);
    
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

DBStringResult BlackLibraryDB::GetStagingEntryUUIDFromUrl(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBStringResult res = database_connection_interface_->GetEntryUUIDFromUrl(url, STAGING_ENTRY);
    if (res.error)
        BlackLibraryCommon::LogError("db", "Failed to get staging UUID from url: {}", url);

    return res;
}

DBStringResult BlackLibraryDB::GetStagingEntryUrlFromUUID(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetEntryUrlFromUUID(uuid, STAGING_ENTRY);
}

DBStringResult BlackLibraryDB::GetBlackEntryUUIDFromUrl(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBStringResult res = database_connection_interface_->GetEntryUUIDFromUrl(url, BLACK_ENTRY);
    if (res.error)
        BlackLibraryCommon::LogError("db", "Failed to get black UUID from url: {}", url);

    return res;
}

DBStringResult BlackLibraryDB::GetBlackEntryUrlFromUUID(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetEntryUrlFromUUID(uuid, BLACK_ENTRY);
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
