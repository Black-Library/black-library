/**
 * WgetUrlPuller.cc
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <WgetUrlPuller.hh>

#include <SQLite3ScraperSanatizer.hh>

namespace black_library {

#define MINIMUM_EXPECTED_URL_LENGTH 13

std::vector<std::string> WgetUrlPuller::PullUrls() const
{
    std::vector<std::string> urls;
    std::ifstream file_stream;
    std::stringstream ss;
    std::string file_line;
    std::string file_name = "black_library_urls";

    ss << "wget https://docs.google.com/document/d/16cnAc7BmSUKsBUdtKny2uQpRezxfDu_n_PIWVVSHsCs/export?format=txt --output-document ";
    ss << file_name;

    std::cout << "Pulling Urls using Wget" << std::endl;

    std::string command = ss.str();

    if (clearenv() != 0)
    {
        std::cout << "Error: could not clear local environment variables" << std::endl;
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
