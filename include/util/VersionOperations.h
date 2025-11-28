/**
 * VersionOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_VERSION_OPERATIONS_H__

#include <string>
#include <sstream>
#include <vector>

#include <JsonOperations.h>

#include <spdlog/fmt/ostr.h>

namespace black_library {

namespace core {

namespace common {

static constexpr const char EmptyMD5Version[] = "NO_MD5_VERSION";
// static const size_t MaxSeqNum = (size_t) - 1;
// max size of int in sqlite is 4 bytes
static const int64_t MaxSeqNum = INT64_MAX;

typedef int64_t seq_num_rep_t;

struct Md5Sum {
    std::string uuid = "";
    std::string md5_sum = EmptyMD5Version;
    size_t index_num = 0;
    time_t date = 0;
    std::string sec_id = "";
    seq_num_rep_t seq_num = MaxSeqNum;
    size_t version_num = 0;
};

inline std::ostream& operator<< (std::ostream &out, const Md5Sum &sum)
{
    out << "UUID: " << sum.uuid << " ";
    out << "md5_sum: " << sum.md5_sum << " ";
    out << "index_num: " << sum.index_num << " ";
    out << "date: " << sum.date << " ";
    out << "sec_id: " << sum.sec_id << " ";
    out << "seq_num: " << sum.seq_num << " ";
    out << "version_number: " << sum.version_num;

    return out;
}

inline bool operator < (const Md5Sum &left, const Md5Sum &right)
{
    return left.index_num < right.index_num;
}

struct Md5SumGreaterThanBySeqNum
{
    bool operator()(const Md5Sum& lhs, const Md5Sum& rhs) const
    {
        return lhs.seq_num > rhs.seq_num;
    }
};

enum class DBMd5SumColumnID : uint8_t
{
    uuid,
    md5_sum,
    index_num,
    date,
    sec_id,
    seq_num,
    version_num,

    _NUM_DB_MD5SUM_COLUMN_ID
};

void from_json(const njson& j, Md5Sum &md5_sum);
void to_json(njson &j, const Md5Sum &md5_sum);

enum class version_extract_t
{
    CHAPTER_SEQ_NUM,
    WORK_NUM,

    _NUM_VERSION_EXTRACT_TYPE
};

std::string GetMD5Hash(const std::string &input);
std::string GetWorkChapterSecIdFromUrl(const std::string &url);
seq_num_rep_t GetWorkChapterSeqNumFromUrl(const std::string &url);
size_t GetWorkNumFromUrl(const std::string &url);

} // namespace common
} // namespace core
} // namespace black_library

template <>
struct fmt::formatter<black_library::core::common::Md5Sum> : fmt::formatter<std::string> {
    auto format(black_library::core::common::Md5Sum md5_sum, format_context &ctx) const {
        std::stringstream ss;
        ss << md5_sum;
        return fmt::format_to(ctx.out(), "{}", ss.str());
    }
};

#endif
