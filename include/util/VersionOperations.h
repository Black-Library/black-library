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
static const size_t MaxIdentifier = (size_t) - 1;

struct Md5Sum {
    std::string uuid = "";
    size_t index_num = 0;
    std::string md5_sum = EmptyMD5Version;
    time_t date = 0;
    std::string identifier = "";
    size_t version_num = 0;
};

inline std::ostream& operator<< (std::ostream &out, const Md5Sum &sum)
{
    out << "UUID: " << sum.uuid << " ";
    out << "index_num: " << sum.index_num << " ";
    out << "md5_sum: " << sum.md5_sum << " ";
    out << "date: " << sum.date << " ";
    out << "identifier: " << sum.identifier << " ";
    out << "version_number: " << sum.version_num;

    return out;
}

inline bool operator < (const Md5Sum &left, const Md5Sum &right)
{
    return left.index_num < right.index_num;
}

struct Md5SumLessThanByIdentifier
{
    bool operator()(const Md5Sum& lhs, const Md5Sum& rhs) const
    {
        return std::stoi(lhs.identifier) < std::stoi(rhs.identifier);
    }
};

enum class DBMd5SumColumnID : uint8_t
{
    uuid,
    index_num,
    md5_sum,
    date,
    identifier,
    version_num,

    _NUM_DB_MD5SUM_COLUMN_ID
};

enum class version_extract_t
{
    CHAPTER,
    WORK,

    _NUM_VERSION_EXTRACT_TYPE
};

std::string GetMD5Hash(const std::string &input);
std::string GetWorkChapterIdentifierFromUrl(const std::string &url);
std::string GetWorkIdentifierFromUrl(const std::string &url);

} // namespace common
} // namespace core
} // namespace black_library

#endif
