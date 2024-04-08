/**
 * CurlOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__

#include <string>

#include <curl/curl.h>

namespace black_library {

namespace core {

namespace common {

std::string CurlGet(const std::string &url);
std::string CurlPost(const std::string &url, const std::string &request);
std::string CurlPut(const std::string &url, const std::string &request);
std::string CurlDelete(const std::string &url, const std::string &request);

} // namespace common
} // namespace core
} // namespace black_library

#endif
