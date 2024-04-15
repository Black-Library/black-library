/**
 * CurlOperations.cc
 */

#include <CurlOperations.h>

namespace black_library {

namespace core {

namespace common {

CurlNetworkAdapter::CurlNetworkAdapter(const njson &config)
{
    njson nconfig = LoadConfig(config);

    std::string logger_path = DefaultLogPath;
    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    bool logger_level = DefaultLogLevel;
    if (nconfig.contains("network_adapter_debug_log"))
    {
        logger_level = nconfig["network_adapter_debug_log"];
    }

    curl_ = curl_easy_init();

    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_body_);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 15L);

    response_body_.reserve(CURL_MAX_WRITE_SIZE);
}

CurlNetworkAdapter::~CurlNetworkAdapter()
{
    curl_easy_cleanup(curl_);
}

void CurlNetworkAdapter::CurlGet(const std::string &url)
{
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);
}

void CurlNetworkAdapter::CurlPost(const std::string &url, const std::string &request)
{
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);
}

void CurlNetworkAdapter::CurlPut(const std::string &url, const std::string &request)
{
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);
}

void CurlNetworkAdapter::CurlDelete(const std::string &url, const std::string &request)
{
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);
}

std::string CurlNetworkAdapter::GetResponseBody()
{
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);
    return response_body_;
}

long CurlNetworkAdapter::GetResponseCode()
{
    return response_code_;
}

std::string CurlNetworkAdapter::GetContentType()
{
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_content_type_);
    return last_content_type_;
}

double CurlNetworkAdapter::GetTotalTime()
{
    double total_time_double;
    curl_easy_getinfo(curl_, CURLINFO_TOTAL_TIME, &total_time_double);
    return total_time_double;
}


std::string CurlGet(const std::string &url)
{
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if (!curl)
        return "";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HandleCurlResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {

        BlackLibraryCommon::LogError(logger_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return "";
    }

    curl_easy_cleanup(curl);

    return response;
}

std::string CurlPost(const std::string &url, const std::string &request);
std::string CurlPut(const std::string &url, const std::string &request);
std::string CurlDelete(const std::string &url, const std::string &request);

} // namespace common
} // namespace core
} // namespace black_library
