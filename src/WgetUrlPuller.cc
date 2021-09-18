/**
 * WgetUrlPuller.cc
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <LogOperations.h>
#include <SQLite3ScraperSanatizer.hh>

#include <WgetUrlPuller.h>

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;

#define MINIMUM_EXPECTED_URL_LENGTH 13

std::vector<std::string> WgetUrlPuller::PullUrls() const
{
    std::vector<std::string> urls;
    std::ifstream file_stream;
    std::stringstream ss;
    std::string file_line;
    const auto file_name = "black_library_urls";

    // prod url
    const auto doc_url = "https://docs.google.com/document/d/1kSsAoUKg6aiXHb_sksAM5qpcCLYNS9n8-MfSgTg9XAY";
    
    // test url
    // const auto doc_url = "https://docs.google.com/document/d/16cnAc7BmSUKsBUdtKny2uQpRezxfDu_n_PIWVVSHsCs";

    // TODO: check if connected to internet first

    ss << "wget --quiet " << doc_url << "/export?format=txt --output-document ";
    ss << file_name;

    BlackLibraryCommon::LogDebug("black_library", "Pulling Urls using Wget");

    const auto command = ss.str();

    if (clearenv() != 0)
    {
        BlackLibraryCommon::LogWarn("black_library", "WgetUrlPuller could not clear local environment variables");
    }

    // TODO: consider not using system 
    // https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=87152177
    int ret = system(command.c_str());

    if (ret < 0)
        return urls;

    file_stream.open(file_name);

    if (!file_stream.is_open())
    {
        file_stream.close();
        return urls;
    }

    while (getline(file_stream, file_line))
    {
        scraper_sanatizer::SQLite3ScraperSanatize(file_line);

        if (file_line.size() <= MINIMUM_EXPECTED_URL_LENGTH)
            continue;
        urls.emplace_back(file_line);
    }
    file_stream.close();

    return urls;
}

} // namespace black_library
