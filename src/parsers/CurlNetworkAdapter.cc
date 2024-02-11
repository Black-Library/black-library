/**
 * CurlNetworkAdapter.cc
 */

#include <curl/curl.h>

#include <LogOperations.h>

#include <CurlNetworkAdapter.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

CurlNetworkAdapter::CurlNetworkAdapter(const std::string &logger_name) :
    logger_name_(logger_name)
{
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

std::string CurlNetworkAdapter::RequestUrl(const std::string url) const
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (!curl)
    {
        BlackLibraryCommon::LogError(logger_name_, "Curl request failed, curl did not intialize");
        return "";
    }

    std::string html_raw;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HandleCurlResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_raw);
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

    return html_raw;
}

} // namespace parsers
} // namespace core
} // namespace black_library
