/**
 * WgetUrlPuller.cc
 */

#include <iostream>

#include <WgetUrlPuller.hh>

namespace black_library {

WgetUrlPuller::WgetUrlPuller()
{

}

std::vector<std::string> WgetUrlPuller::PullUrls() const
{
    std::vector<std::string> urls;

    std::cout << "Pulling urls using wget" << std::endl;

    return urls;
}

} // namespace black_library
