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
    if (nconfig.contains("parser_db_adapter_debug_log"))
    {
        logger_level = nconfig["parser_db_adapter_debug_log"];
    }

    if (nconfig.contains("db_version"))
    {
        db_version = nconfig["db_version"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);
}

ParserVersionCheckResult ParserDbAdapter::CheckVersion(const std::string &content, const std::string &uuid, const size_t index_num, const time_t time)
{
    ParserVersionCheckResult version_check;
    auto content_md5 = BlackLibraryCommon::GetMD5Hash(content);
    BlackLibraryCommon::LogDebug(logger_name_, "CheckVersion UUID: {} index: {} checksum hash: {}", uuid, index_num, content_md5);

    BlackLibraryCommon::Md5Sum md5_check = CheckForMd5(content_md5, uuid);

    BlackLibraryCommon::LogDebug(logger_name_, "CheckVersion UUID: {} checksum hash: {}", uuid, md5_check.md5_sum);

    if (md5_check.md5_sum == content_md5 && md5_check.date == time)
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

BlackLibraryCommon::Md5Sum ParserDbAdapter::ReadMd5BySecId(const std::string &uuid, const std::string &sec_id)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
        return md5;
    }

    if (!blacklibrary_db_->DoesMd5SumExistBySecId(uuid, sec_id))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} sec_id: {} does not exist for md5 read", uuid, sec_id);
        return md5;
    }

    return  blacklibrary_db_->ReadMd5SumBySecId(uuid, sec_id);
}

BlackLibraryCommon::Md5Sum ParserDbAdapter::ReadMd5BySeqNum(const std::string &uuid, const size_t &seq_num)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
        return md5;
    }

    if (!blacklibrary_db_->DoesMd5SumExistBySeqNum(uuid, seq_num))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} seq_num: {} does not exist for md5 read", uuid, seq_num);
        return md5;
    }

    return  blacklibrary_db_->ReadMd5SumBySeqNum(uuid, seq_num);
}

BlackLibraryCommon::Md5Sum ParserDbAdapter::ReadMd5ByUrl(const std::string &uuid, const std::string &url)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
        return md5;
    }

    size_t seq_num = BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(url);
    bool md5_exist_seq_num = blacklibrary_db_->DoesMd5SumExistBySeqNum(uuid, seq_num);
    if (md5_exist_seq_num)
        return blacklibrary_db_->ReadMd5SumBySeqNum(uuid, seq_num);

    std::string sec_id = BlackLibraryCommon::GetWorkChapterSecIdFromUrl(url);
    bool md5_exist_sec_id = blacklibrary_db_->DoesMd5SumExistBySecId(uuid, sec_id);
    if (md5_exist_sec_id)
        return blacklibrary_db_->ReadMd5SumBySecId(uuid, sec_id);

    if (!md5_exist_seq_num && !md5_exist_sec_id)
        BlackLibraryCommon::LogDebug(logger_name_, "Read md5 UUID: {} failed md5 sum does not exist for url: {}", uuid, url);

    return md5;
}

std::vector<BlackLibraryCommon::Md5Sum> ParserDbAdapter::ReadMd5s(const std::string &uuid)
{
    return blacklibrary_db_->GetMd5SumsFromUUID(uuid);
}

int ParserDbAdapter::UpsertMd5ByIndexNum(const std::string &uuid, size_t index_num, const std::string &md5_sum, const std::string &url, time_t date, uint64_t version_num)
{
    const std::lock_guard<std::mutex> lock(upsert_mutex_);

    std::string sec_id = BlackLibraryCommon::GetWorkChapterSecIdFromUrl(url);
    BlackLibraryCommon::seq_num_rep_t seq_num = BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(url);

    BlackLibraryCommon::Md5Sum md5 = { uuid, md5_sum, index_num, date, sec_id, seq_num, version_num };

    // if (blacklibrary_db_->DoesMd5SumExist())
    // {
    //     BlackLibraryCommon::LogWarn(logger_name_, "Exact copy of md5 UUID: {} index_num: {} md5_sum: {} date: {} sec_id: {} seq_num: {} already exists", uuid, index_num, md5_sum, date, sec_id, seq_num);
    //     return;
    // }

    if (blacklibrary_db_->DoesMd5SumExistByIndexNum(uuid, index_num))
    {
        if (blacklibrary_db_->UpdateMd5SumByIndexNum(md5))
        {
            BlackLibraryCommon::LogError(logger_name_, "Update md5 UUID: {} index_num: {} md5_sum: {} date: {} sec_id: {} seq_num: {} failed", uuid, index_num, md5_sum, date, sec_id, seq_num);
            return -1;
        }
        
        return 0;
    }

    // otherwise, create a new one
    if (blacklibrary_db_->CreateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Create md5 UUID: {} index_num: {} md5_sum: {} date: {} sec_id: {} seq_num: {} failed", uuid, index_num, md5_sum, date, sec_id, seq_num);
        return -1;
    }

    return 0;
}

int ParserDbAdapter::UpdateMd5BySeqNum(const BlackLibraryCommon::Md5Sum &md5_sum)
{
    const std::lock_guard<std::mutex> lock(upsert_mutex_);

    if (!blacklibrary_db_->DoesMd5SumExistBySeqNum(md5_sum.uuid, md5_sum.seq_num))
    {
        BlackLibraryCommon::LogError(logger_name_, "UpdateMd5BySeqNum could not update, md5 does not exist");
        return -1;
    }

    if (blacklibrary_db_->UpdateMd5SumBySeqNum(md5_sum))
    {
        BlackLibraryCommon::LogError(logger_name_, "Update md5 UUID: {} index_num: {} md5_sum: {} date: {} sec_id: {} seq_num: {} failed", md5_sum.uuid, md5_sum.index_num, md5_sum.md5_sum, md5_sum.date, md5_sum.sec_id, md5_sum.seq_num);
        return -1;
    }

    return 0;
}

} // namespace parsers
} // namespace core
} // namespace black_library
