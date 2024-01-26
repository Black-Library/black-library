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
    logger_name_("db"),
    mutex_()
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);
    std::string db_version = "";

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

    if (nconfig.contains("db_version"))
    {
        db_version = nconfig["db_version"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);

    database_connection_interface_ = std::make_unique<SQLiteDB>(database_url, db_version);
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

std::vector<BlackLibraryCommon::Md5Sum> BlackLibraryDB::GetChecksumList()
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
        BlackLibraryCommon::LogError(logger_name_, "Failed to create entry with UUID: {}", entry.uuid);
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
        BlackLibraryCommon::LogError(logger_name_, "Failed to read entry with empty UUID");
        return entry;
    }

    entry = database_connection_interface_->ReadEntry(uuid);

    if (entry.uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read entry with UUID: {}", uuid);
        return entry;
    }

    return entry;
}

int BlackLibraryDB::UpdateWorkEntry(const DBEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->UpdateEntry(entry))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteWorkEntry(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete entry with empty UUID");
        return -1;
    }

    if (database_connection_interface_->DeleteEntry(uuid))

    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete entry with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateMd5Sum(const BlackLibraryCommon::Md5Sum &md5)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (md5.uuid.empty() || database_connection_interface_->CreateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to create MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);
        return -1;
    }

    return 0;
}

BlackLibraryCommon::Md5Sum BlackLibraryDB::ReadMd5SumByIndexNum(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    BlackLibraryCommon::Md5Sum md5;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with empty UUID");
        return md5;
    }
    md5 = database_connection_interface_->ReadMd5SumByIndexNum(uuid, index_num);
    if (md5.uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with UUID: {} index_num: {}", uuid, index_num);
        return md5;
    }

    return md5;
}

BlackLibraryCommon::Md5Sum BlackLibraryDB::ReadMd5SumBySecId(const std::string &uuid, const std::string &sec_id)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    BlackLibraryCommon::Md5Sum md5;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with empty UUID");
        return md5;
    }
    md5 = database_connection_interface_->ReadMd5SumBySecId(uuid, sec_id);
    if (md5.uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with UUID: {} sec_id: {}", uuid, sec_id);
        return md5;
    }

    return md5;
}

BlackLibraryCommon::Md5Sum BlackLibraryDB::ReadMd5SumBySeqNum(const std::string &uuid, const size_t &seq_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    BlackLibraryCommon::Md5Sum md5;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with empty UUID");
        return md5;
    }
    md5 = database_connection_interface_->ReadMd5SumBySeqNum(uuid, seq_num);
    if (md5.uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read MD5 checksum with UUID: {} seq_num: {}", uuid, seq_num);
        return md5;
    }

    return md5;
}

int BlackLibraryDB::UpdateMd5SumByIndexNum(const BlackLibraryCommon::Md5Sum &md5)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (md5.uuid.empty() || database_connection_interface_->UpdateMd5SumByIndexNum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::UpdateMd5SumBySeqNum(const BlackLibraryCommon::Md5Sum &md5)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (md5.uuid.empty() || database_connection_interface_->UpdateMd5SumBySeqNum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update MD5 checksum with UUID: {} index_num: {} sum: {}", md5.uuid, md5.index_num, md5.md5_sum);
        return -1;
    }

    return 0;
}


int BlackLibraryDB::DeleteMd5Sum(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete MD5 checksum with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteMd5Sum(uuid, index_num))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete MD5 checksum with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateRefresh(const DBRefresh &refresh)
{
    if (refresh.uuid.empty() || database_connection_interface_->CreateRefresh(refresh))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to create refresh with UUID: {} refresh_date: {}", refresh.uuid, refresh.refresh_date);
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
        BlackLibraryCommon::LogError(logger_name_, "Failed to read refresh with empty UUID");
        return refresh;
    }
    refresh = database_connection_interface_->ReadRefresh(uuid);
    if (refresh.uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to read refresh with UUID: {}", uuid);
        return refresh;
    }

    return refresh;
}

int BlackLibraryDB::DeleteRefresh(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete refresh with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteRefresh(uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete refresh with UUID: {}", uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::CreateErrorEntry(const DBErrorEntry &entry)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (entry.uuid.empty() || database_connection_interface_->CreateErrorEntry(entry))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to create error entry with UUID: {}", entry.uuid);
        return -1;
    }

    return 0;
}

int BlackLibraryDB::DeleteErrorEntry(const std::string &uuid, size_t progress_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete error entry with empty UUID");
        return -1;
    }
    if (database_connection_interface_->DeleteErrorEntry(uuid, progress_num))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete error entry with UUID: {} and progress number: {}", uuid, progress_num);
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
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
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
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesMd5SumExistByIndexNum(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesMd5SumExistByIndexNum(uuid, index_num);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesMd5SumExistBySecId(const std::string &uuid, const std::string &sec_id)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesMd5SumExistBySecId(uuid, sec_id);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

bool BlackLibraryDB::DoesMd5SumExistBySeqNum(const std::string &uuid, const size_t &seq_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBBoolResult check = database_connection_interface_->DoesMd5SumExistBySeqNum(uuid, seq_num);
    
    if (check.error != 0)
    {
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
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
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
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
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
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
        BlackLibraryCommon::LogError(logger_name_, "Database returned {}", check.error);
        return false;
    }

    return check.result;
}

DBStringResult BlackLibraryDB::GetDBVersion()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBStringResult res = database_connection_interface_->GetDBVersion();
    if (res.error)
        BlackLibraryCommon::LogError(logger_name_, "Failed to get db version");

    return res;
}

DBStringResult BlackLibraryDB::GetWorkEntryUUIDFromUrl(const std::string &url)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    DBStringResult res = database_connection_interface_->GetEntryUUIDFromUrl(url);
    if (res.error)
        BlackLibraryCommon::LogError(logger_name_, "Failed to get UUID from url: {}", url);

    return res;
}

DBStringResult BlackLibraryDB::GetWorkEntryUrlFromUUID(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetEntryUrlFromUUID(uuid);
}

BlackLibraryCommon::Md5Sum BlackLibraryDB::GetMd5SumFromMd5Sum(const std::string &md5_sum, const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    BlackLibraryCommon::Md5Sum md5;

    if (md5_sum.empty() || uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to get version from MD5 checksum with empty md5_sum or UUID");
        return md5;
    }

    md5 = database_connection_interface_->GetMd5SumFromMd5Sum(md5_sum, uuid);

    return md5;
}

std::vector<BlackLibraryCommon::Md5Sum> BlackLibraryDB::GetMd5SumsFromUUID(const std::string &uuid)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    return database_connection_interface_->GetMd5SumsFromUUID(uuid);
}

uint16_t BlackLibraryDB::GetVersionFromMd5(const std::string &uuid, size_t index_num)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    uint16_t version_num = 0;

    if (uuid.empty())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to get version from MD5 checksum with empty UUID");
        return version_num;
    }
    version_num = database_connection_interface_->GetVersionFromMd5(uuid, index_num);

    return version_num;
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
