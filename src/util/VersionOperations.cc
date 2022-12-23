/**
 * VersionOperations.cc
 */

#include <iomanip>
#include <sstream>

#include <openssl/md5.h>

#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace common {

std::string GetMD5Hash(const std::string &input)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    std::ostringstream oss;

    MD5((unsigned char*)input.c_str(), input.size(), result);

    oss << std::hex << std::setfill('0');
    for (auto c : result) oss << std::setw(2) << (int)c;

    return oss.str();
}

} // namespace common
} // namespace core
} // namespace black_library
