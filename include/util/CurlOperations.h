/**
 * CurlOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__

#include <string>

#include <curl/curl.h>

#include <ConfigOperations.h>
#include <LogOperations.h>

// https://github.com/PwRAu/scraf-backend/blob/9edcd323b8d174aaf6ce29fd7f5a4456c56624f0/tests/students.cpp#L31C1-L52
// https://github.com/Tachi107/scrafurl/blob/main/include/scrafurl.hpp

namespace black_library {

namespace core {

namespace common {

enum class curl_aio_behavior_t {
    ERROR,
    DIRECT_CURL,
    ADAPTER_CURL
};

typedef bool curl_result_error_rep;

struct CurlResult {
    std::string response_body = "";
    std::string error = "";
    std::string curl_ver = "";
    long response_code = 0;
    double total_time = 0;
    curl_aio_behavior_t func_ver = curl_aio_behavior_t::ERROR;
    curl_result_error_rep has_error = true;
};

std::string GetNetworkAdapterName(curl_aio_behavior_t behavior);

class CurlNetworkAdapter {
public:
    CurlNetworkAdapter(const njson &config);
    ~CurlNetworkAdapter();

    CurlResult CurlGetAllInOne(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior);

    void CurlGet(const std::string &url);
    void CurlPost(const std::string &url, const std::string &request);
    void CurlPut(const std::string &url, const std::string &request);
    void CurlDelete(const std::string &url);

    std::string GetResponseBody();
    long GetResponseCode();

    std::string GetContentType();
    double GetTotalTime();

private:
    CurlResult CurlGetAIO_direct(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior);
    CurlResult CurlGetAIO_adapter(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    CURL* curl_;
    std::string response_body_;
    long response_code_;

    std::string last_content_type_;
    std::string last_error_;

    std::string logger_name_;
    std::mutex mutex_;
};

} // namespace common
} // namespace core
} // namespace black_library

#endif
