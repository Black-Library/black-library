/**
 * ParserDbAdapter.cc
 */

#include <LogOperations.h>

#include <BlackLibraryDB.h>
#include <ParserDbAdapter.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryDB = black_library::core::db;

ParserDbAdapter::ParserDbAdapter(const njson &config, const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db) :
    blacklibrary_db_(blacklibrary_db),
    logger_name_("parser_db_adapter"),
    upsert_mutex_()
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);
    std::string db_version = "";

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
}

// get section content
// check to see if md5 already exists
// if exists, but no index, update index
// if exists with correct index, skip
// if 

ParserVersionCheckResult ParserDbAdapter::CheckVersion(const std::string &content, const std::string &uuid, const size_t index_num, const time_t time, const std::string &url)
{
    ParserVersionCheckResult version_check;
    auto content_md5 = BlackLibraryCommon::GetMD5Hash(content);
    BlackLibraryCommon::LogDebug(logger_name_, "CheckVersion UUID: {} index: {} checksum hash: {}", uuid, index_num, content_md5);

    BlackLibraryCommon::Md5Sum md5_check = CheckForMd5(content_md5, uuid);

    if (md5_check.md5_sum != BlackLibraryCommon::EmptyMD5Version)
    {
        version_check.already_exists = true;
        version_check.has_error = false;

        return version_check;
    }

    version_check.md5 = content_md5;

    version_check.has_error = false;

    return version_check;
}

BlackLibraryCommon::Md5Sum ParserDbAdapter::CheckForMd5(const std::string &md5_sum, const std::string &uuid)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 check", uuid);
        return md5;
    }

    md5 = blacklibrary_db_->GetMd5SumFromMd5Sum(md5_sum, uuid);

    return md5;
}

BlackLibraryCommon::Md5Sum ParserDbAdapter::ReadMd5(const std::string &uuid, const std::string &url)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
        return md5;
    }
    if (!blacklibrary_db_->DoesMd5SumExistUrl(uuid, url))
    {
        BlackLibraryCommon::LogDebug(logger_name_, "Read md5 UUID: {} url: {} failed md5 sum does not exist", uuid, url);
        return md5;
    }

    md5 = blacklibrary_db_->ReadMd5SumUrl(uuid, url);

    return md5;
}

std::unordered_map<std::string, BlackLibraryCommon::Md5Sum> ParserDbAdapter::ReadMd5s(const std::string &uuid)
{
    return blacklibrary_db_->GetMd5SumsFromUUID(uuid);
}

int ParserDbAdapter::UpsertMd5(const std::string &uuid, size_t index_num, const std::string &md5_sum, time_t date, const std::string &url, uint64_t version_num)
{
    const std::lock_guard<std::mutex> lock(upsert_mutex_);

    BlackLibraryCommon::Md5Sum md5 = { uuid, index_num, md5_sum, date, url, version_num };

    // if exact copy already exists print warning, previous step should have already caught
    // if (blacklibrary_db_->DoesMd5SumExistExact())
    // {
    //     BlackLibraryCommon::LogError(logger_name_, "Exact copy of md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} already exists", uuid, index_num, md5_sum, date, url);
    //     return;
    // }

    if (blacklibrary_db_->DoesMd5SumExistIndexNum(uuid, index_num))
    {
        if (blacklibrary_db_->UpdateMd5Sum(md5))
        {
            BlackLibraryCommon::LogError(logger_name_, "Update md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} already exists", uuid, index_num, md5_sum, date, url, version_num);
            return -1;
        }
        
        return 0;
    }

    // otherwise, create a new one
    if (blacklibrary_db_->CreateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Create md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} failed", uuid, index_num, md5_sum, date, url, version_num);
        return -1;
    }

    return 0;
}

} // namespace parsers
} // namespace core
} // namespace black_library
