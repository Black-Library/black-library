/**
 * BlackLibrary.cc
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <BlackLibrary.hh>

namespace black_library {

BlackLibrary::BlackLibrary() :
    blacklibrarydb_("", true),
    parse_urls_(),
    urls_(),
    done_(true)
{
    Init();
}

int BlackLibrary::Init()
{
    std::cout << "Initialize: BlackLibrary" << std::endl;
    return 0;
}

int BlackLibrary::Run()
{
    done_ = false;

    while (!done_)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        
        RunOnce();

        if (done_)
            break;

        std::this_thread::sleep_until(deadline);
    }

    return 0;
}

int BlackLibrary::RunOnce()
{
    return 0;
}

int BlackLibrary::Stop()
{
    done_ = true;

    return 0;
}

} // namespace black_library
