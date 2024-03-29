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

std::vector<std::string> WgetUrlPuller::PullUrls(bool debug_target) const
{
    std::vector<std::string> urls;
    std::ifstream file_stream;
    std::stringstream ss;
    std::string file_line;
    const auto file_name = "black_library_urls";

    // prod url
    auto doc_url = "https://docs.google.com/document/d/1kSsAoUKg6aiXHb_sksAM5qpcCLYNS9n8-MfSgTg9XAY";

    if (debug_target)
        doc_url = "https://docs.google.com/document/d/16cnAc7BmSUKsBUdtKny2uQpRezxfDu_n_PIWVVSHsCs";

    // TODO: check if connected to internet first

    ss << "wget --quiet " << doc_url << "/export?format=txt --output-document ";
    ss << file_name;

    const auto command = ss.str();

    BlackLibraryCommon::LogDebug("black_library", "WgetUrlPuller pulling Urls using Wget with command: {}", command);

    if (clearenv() != 0)
    {
        BlackLibraryCommon::LogWarn("black_library", "WgetUrlPuller failed to clear local environment variables");
    }

    // TODO: consider not using system 
    // https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=87152177
    int ret = system(command.c_str());

    if (ret < 0)
    {
        BlackLibraryCommon::LogError("black_library", "WgetUrlPuller failed wget system call");
        return urls;
    }

    BlackLibraryCommon::LogDebug("black_library", "WgetUrlPuller wget system return value: {}", ret);


    file_stream.open(file_name);

    if (!file_stream.is_open())
    {
        BlackLibraryCommon::LogError("black_library", "WgetUrlPuller failed to open file stream");
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
