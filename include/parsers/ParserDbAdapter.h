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

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

class ParserDbAdapter
{
public:
    explicit ParserDbAdapter(const std::shared_ptr<BlackLibraryDB::BlackLibraryDB> &blacklibrary_db_);

    // size_t CheckVersion(const std::string &content, const std::string &uuid, const size_t index_num);
    // size_t GetVersion();
    // size_t UpsertVersion();

    BlackLibraryCommon::Md5Sum CheckForMd5(const std::string &md5_sum, const std::string &uuid);
    BlackLibraryCommon::Md5Sum ReadMd5(const std::string &uuid, const std::string &url);
    std::unordered_map<std::string, BlackLibraryCommon::Md5Sum> ReadMd5s(const std::string &uuid);
    void UpsertMd5(const std::string &uuid, size_t index_num, const std::string &md5_sum, time_t date, const std::string &url, uint64_t version_num);

private:
    std::shared_ptr<BlackLibraryDB::BlackLibraryDB> blacklibrary_db_;

    std::string logger_name_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
