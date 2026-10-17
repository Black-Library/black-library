/**
 * CurlOperations.cc
 */

#include <string>

#include <CurlOperations.h>

namespace black_library {

namespace core {

namespace common {

std::string GetNetworkAdapterName(curl_aio_behavior_t behavior)
{
    switch (behavior)
    {
    case curl_aio_behavior_t::ERROR:
        return "ERROR_BEHAVIOR";
        break;
    case curl_aio_behavior_t::DIRECT_CURL:
        return "DIREC_CURL_BEHAVIOR";
        break;
    case curl_aio_behavior_t::ADAPTER_CURL:
        return "ADAPTER_CURL_BEHAVIOR";
        break;
    default:
        return "NO_BEHAVIOR";
        break;
    }
}

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
    last_error_(""),
    logger_name_("curl_network_adapter"),
    mutex_()
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

    if (!curl_)
    {
        LogError(logger_name_, "Could not Initialize curl network adapter");
    }

    // curl verbose output spams the std out log
    // curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, "/mnt/black-library/cookies/cookies.txt");
    curl_easy_setopt(curl_, CURLOPT_COOKIEJAR, "/mnt/black-library/cookies/cookies.txt");
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_body_);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 15L);

    response_body_.reserve(CURL_MAX_WRITE_SIZE);
}

CurlNetworkAdapter::~CurlNetworkAdapter()
{
    curl_easy_cleanup(curl_);
}

CurlResult CurlNetworkAdapter::CurlGetAllInOne(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior)
{
    LogDebug(logger_name_, "Curl AIO access url: {} behavior: {}", url, GetNetworkAdapterName(curl_aio_behavior));
    switch (curl_aio_behavior)
    {
    case curl_aio_behavior_t::DIRECT_CURL:
        return CurlGetAIO_direct(url, curl_aio_behavior);
        break;
    case curl_aio_behavior_t::ADAPTER_CURL:
        return CurlGetAIO_adapter(url, curl_aio_behavior);
        break;
    default:
        LogError(logger_name_, "Could not find curl behavior {}", GetNetworkAdapterName(curl_aio_behavior));
        break;
    }

    CurlResult curl_res;

    return curl_res;
}

void CurlNetworkAdapter::CurlGet(const std::string &url)
{
    response_body_.clear();
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        std::string error_msg = curl_easy_strerror(res);
        LogError(logger_name_, "Curl request failed: {}", error_msg);
        last_error_ = error_msg;
        return;
    }
    else
    {
        // save cookies
        CURLcode cookie_res = curl_easy_setopt(curl_, CURLOPT_COOKIELIST, "FLUSH");
        if (cookie_res != CURLE_OK)
        {
            LogError(logger_name_, "Failed to flush cookies");
        }
    }
}

void CurlNetworkAdapter::CurlPost(const std::string &url, const std::string &request)
{
    response_body_.clear();

    curl_mime *multipart = curl_mime_init(curl_);
    if (!multipart)
    {
        LogError(logger_name_, "Could not create curl mime init");
    }
    curl_mimepart *part = curl_mime_addpart(multipart);
    // TODO: update post
    curl_mime_name(part, "example-name");
    curl_mime_data(part, "example-data", CURL_ZERO_TERMINATED);
    curl_easy_setopt(curl_, CURLOPT_MIMEPOST, multipart);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, &request);

    CURLcode res = curl_easy_perform(curl_);
    curl_mime_free(multipart);

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

/**
 * Returns the CurlNetworkAdapter response body.
 *
 * @return The HTTP response body.
 */
std::string CurlNetworkAdapter::GetResponseBody()
{
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);
    return response_body_;
}

/**
 * Returns the HTTP Status Code for the most recent curl operation.
 *
 * @return The HTTP Status Code for the most recent curl operation.
 */
long CurlNetworkAdapter::GetResponseCode()
{
    return response_code_;
}

std::string CurlNetworkAdapter::GetContentType()
{
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_content_type_);
    return last_content_type_;
}

/**
 * Returns the total curl time taken for the most recent curl operation.
 *
 * @return total time.
 */
double CurlNetworkAdapter::GetTotalTime()
{
    double total_time_double;
    curl_easy_getinfo(curl_, CURLINFO_TOTAL_TIME, &total_time_double);
    return total_time_double;
}

CurlResult CurlNetworkAdapter::CurlGetAIO_direct(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior)
{
    CurlResult curl_res;

    CURL* curl;
    CURLcode res;
    std::string response;
    long response_code;
    double total_time_double;

    curl = curl_easy_init();

    if (!curl)
        return curl_res;
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
        curl_res.error = curl_easy_strerror(res);
        return curl_res;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_getinfo(curl_, CURLINFO_TOTAL_TIME, &total_time_double);

    curl_easy_cleanup(curl);

    curl_res.response_body = response;
    curl_res.curl_ver = curl_version();
    curl_res.response_code = response_code;
    curl_res.total_time = total_time_double;
    curl_res.func_ver = curl_aio_behavior;
    curl_res.has_error = false;

    return curl_res;
}

CurlResult CurlNetworkAdapter::CurlGetAIO_adapter(const std::string &url, const curl_aio_behavior_t &curl_aio_behavior)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    CurlResult res;

    CurlGet(url);
    res.response_body = GetResponseBody();
    res.response_code = GetResponseCode();
    res.total_time = GetTotalTime();
    res.curl_ver = curl_version();
    res.func_ver = curl_aio_behavior;

    if (res.response_code >= 400)
    {
        res.error = last_error_;
    }
    else
        res.has_error = false;
    
    return res;
}

size_t CurlNetworkAdapter::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;

    // Guard against a null user pointer to prevent segmentation faults
    if (userp == nullptr) {
        return 0; // Returning 0 tells libcurl to abort the transfer
    }

    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total);

    return total;
}

} // namespace common
} // namespace core
} // namespace black_library
