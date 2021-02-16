/**
 * WgetUrlPuller.cc
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <WgetUrlPuller.hh>

namespace black_library {

WgetUrlPuller::WgetUrlPuller()
{

}

std::vector<std::string> WgetUrlPuller::PullUrls() const
{
    std::vector<std::string> urls;
    std::ifstream file_stream;
    std::stringstream ss;
    std::string file_line;
    std::string file_name = "black_library_urls";

    ss << "wget https://docs.google.com/document/d/1kSsAoUKg6aiXHb_sksAM5qpcCLYNS9n8-MfSgTg9XAY/export?format=txt --output-document ";
    ss << file_name;

    std::cout << "Pulling urls using wget" << std::endl;

    std::string command = ss.str();

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
        urls.emplace_back(file_line);
    }
    file_stream.close();

    return urls;
}

} // namespace black_library
