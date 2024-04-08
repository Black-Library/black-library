/**
 * CurlOperations.cc
 */

#include <CurlOperations.h>

namespace black_library {

namespace core {

namespace common {

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
