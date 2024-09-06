/**
 * CurlOperations.cc
 */

#include <CurlOperations.h>

namespace black_library {

namespace core {

namespace common {

// Credit: https://stackoverflow.com/questions/5525613/how-do-i-fetch-a-html-page-source-with-libcurl-in-c
size_t HandleCurlResponse(void* ptr, size_t size, size_t nmemb, void* data)
{
    std::string* str = (std::string*) data;
    char* sptr = (char*) ptr;

    for (size_t x = 0; x < size * nmemb; ++x)
    {
        (*str) += sptr[x];
    }

    return size * nmemb;
}

CurlNetworkAdapter::CurlNetworkAdapter(const njson &config) :
    curl_(NULL),
    response_body_(""),
    response_code_(0),
    last_content_type_(""),
    logger_name_("curl_network_adapter")
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

    InitRotatingLogger(logger_name_, logger_path, logger_level);

    LogDebug(logger_name_, "Initializing curl network adapter");

    curl_ = curl_easy_init();

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
    response_body_.clear();
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        LogError(logger_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return;
    }
}

void CurlNetworkAdapter::CurlPost(const std::string &url, const std::string &request)
{
    response_body_.clear();
    curl_easy_setopt(curl_, CURLOPT_HTTPPOST, 1L);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, &request);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        LogError(logger_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return;
    }
}

void CurlNetworkAdapter::CurlPut(const std::string &url, const std::string &request)
{
    response_body_.clear();
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, &request);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        LogError(logger_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return;
    }
}

void CurlNetworkAdapter::CurlDelete(const std::string &url)
{
    response_body_.clear();
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        LogError(logger_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return;
    }
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


std::string CurlNetworkAdapter::RequestUrl(const std::string &url)
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
        LogError("black_library", "Curl request failed: {}", curl_easy_strerror(res));
        return "";
    }

    curl_easy_cleanup(curl);

    return response;
}

} // namespace common
} // namespace core
} // namespace black_library
