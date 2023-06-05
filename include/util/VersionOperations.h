/**
 * VersionOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__

#include <string>
#include <sstream>
#include <vector>

namespace black_library {

namespace core {

namespace common {

static constexpr const char EmptyMD5Version[] = "NO_MD5_VERSION";

struct Md5Sum {
    std::string uuid = "";
    size_t index_num = 0;
    std::string md5_sum = EmptyMD5Version;
    time_t date = 0;
    std::string url = "";
    size_t version_num = 0;
};

inline std::ostream& operator<< (std::ostream &out, const Md5Sum &sum)
{
    out << "UUID: " << sum.uuid << " ";
    out << "index_num: " << sum.index_num << " ";
    out << "md5_sum: " << sum.md5_sum << " ";
    out << "date: " << sum.date << " ";
    out << "url: " << sum.url << " ";
    out << "version_number: " << sum.version_num;

    return out;
}

inline bool operator < (const Md5Sum &left, const Md5Sum &right)
{
    return left.index_num < right.index_num;
}

enum class DBMd5SumColumnID : uint8_t
{
    uuid,
    index_num,
    md5_sum,
    date,
    url,
    version_num,

    _NUM_DB_MD5SUM_COLUMN_ID
};

std::string GetMD5Hash(const std::string &input);

} // namespace common
} // namespace core
} // namespace black_library

#endif
