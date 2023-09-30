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

ParserDbAdapter::ParserDbAdapter(const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db) :
    blacklibrary_db_(blacklibrary_db),
    logger_name_("parser_db_adapter")
{
}

// get section content
// check to see if md5 already exists
// if exists, but no index, update index
// if exists with correct index, 
// if 

// size_t ParserDbAdapter::CheckVersion(const std::string &content, const std::string &uuid, const size_t index_num)
// {
//     auto content_md5 = BlackLibraryCommon::GetMD5Hash(content);
//     BlackLibraryCommon::LogDebug(logger_name_, "Section UUID: {} index: {} checksum hash: {}", uuid, index_num, content_md5);

//     BlackLibraryCommon::Md5Sum md5_check;
//     if (md5_check_callback_)
//         md5_check = md5_check_callback_(content_md5, uuid);

//     // md5 already exists with correct md5_sum, update md5 with the correct index_num
//     if (md5_check.md5_sum != BlackLibraryCommon::EmptyMD5Version)
//     {
//         BlackLibraryCommon::LogDebug(logger_name_, "Version hash matches: {} index: {}, skip file save", uuid, md5_check.index_num);
//         index_num = md5_check.index_num;

//         if (md5_upsert_callback_)
//             md5_upsert_callback_(uuid, index_num, content_md5, index_entry.time_published, index_entry.data_url, 0);

//         output.has_error = false;

//         return output;
//     }

//     return 15;
// }

BlackLibraryCommon::Md5Sum ParserDbAdapter::CheckForMd5(const std::string &md5_sum, const std::string &uuid)
{
    BlackLibraryCommon::Md5Sum md5;
    if (!blacklibrary_db_->DoesWorkEntryUUIDExist(uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
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

void ParserDbAdapter::UpsertMd5(const std::string &uuid, size_t index_num, const std::string &md5_sum, time_t date, const std::string &url, uint64_t version_num)
{
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
            return;
        }
        
        return;
    }

    // otherwise, create a new one
    if (blacklibrary_db_->CreateMd5Sum(md5))
    {
        BlackLibraryCommon::LogError(logger_name_, "Create md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} failed", uuid, index_num, md5_sum, date, url, version_num);
        return;
    }
}

// size_t ParserDbAdapter::UpsertVersion(const std::string &uuid,)
// {
//     if (md5_upsert_callback_)
//         md5_upsert_callback_(uuid_, index_num, section_md5, index_entry.time_published, index_entry.data_url, version_num);
// }

} // namespace parsers
} // namespace core
} // namespace black_library
