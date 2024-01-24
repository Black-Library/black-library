/**
 * ParserDbAdapter.h
*/

#ifndef __BLACK_LIBRARY_CORE_PARSERS_db_adapter_H__
#define __BLACK_LIBRARY_CORE_PARSERS_db_adapter_H__

#include <functional>
#include <string>
#include <unordered_map>

#include <VersionOperations.h>

#include <BlackLibraryDB.h>
#include <ParserCommon.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

class ParserDbAdapter
{
public:
    explicit ParserDbAdapter(const njson &config, const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db_);

    ParserVersionCheckResult CheckVersion(const std::string &content, const std::string &uuid, const size_t index_num, const time_t time);
    // size_t GetVersion();
    // size_t UpsertVersion();

    BlackLibraryCommon::Md5Sum CheckForMd5(const std::string &md5_sum, const std::string &uuid);
    BlackLibraryCommon::Md5Sum ReadMd5BySecId(const std::string &uuid, const std::string &sec_id);
    BlackLibraryCommon::Md5Sum ReadMd5BySeqNum(const std::string &uuid, const size_t &seq_num);
    BlackLibraryCommon::Md5Sum ReadMd5ByUrl(const std::string &uuid, const std::string &url);
    std::vector<BlackLibraryCommon::Md5Sum> ReadMd5s(const std::string &uuid);
    int UpsertMd5(const std::string &uuid, size_t index_num, const std::string &md5_sum, const std::string &surl, time_t date, uint64_t version_num);

private:
    std::shared_ptr<BlackLibraryDB::BlackLibraryDB> blacklibrary_db_;

    std::string logger_name_;
    std::mutex upsert_mutex_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
