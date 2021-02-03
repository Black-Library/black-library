/**
 * BlackLibrary.cc
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <BlackLibrary.hh>
#include <WgetUrlPuller.hh>

namespace black_library {

BlackLibrary::BlackLibrary(const std::string &db_url) :
    blacklibrarydb_(db_url, true),
    url_puller_(nullptr),
    parse_urls_(),
    urls_(),
    done_(true)
{
    Init();
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
    urls_.clear();
    parse_urls_.clear();

    if (PullUrls())
    {
        std::cout << "Error: Pulling Urls failed" << std::endl;
        return -1;
    }

    if (CompareUrls())
    {
        std::cout << "Error Comparing Urls failed" << std::endl;
        return -1;
    }

    UpdateUrls();
    UpdateStaging();
    ParseUrls();
    UpdateEntries();
    CleanStaging();

    return 0;
}

int BlackLibrary::Stop()
{
    done_ = true;

    std::cout << "Stopping BlackLibrary" << std::endl;

    return 0;
}

int BlackLibrary::Init()
{
    std::cout << "Initializing BlackLibrary" << std::endl;

    url_puller_ = std::make_shared<WgetUrlPuller>();

    return 0;
}

int BlackLibrary::PullUrls()
{
    std::cout << "Pulling Urls from source" << std::endl;

    return 0;
}

int BlackLibrary::CompareUrls()
{
    std::cout << "Comparing Urls with database" << std::endl;

    return 0;
}

int BlackLibrary::UpdateUrls()
{
    std::cout << "Updating Urls to for staging and parsing" << std::endl;

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    std::cout << "Update staging tables of database" << std::endl;

    return 0;
}

int BlackLibrary::ParseUrls()
{
    std::cout << "Parsing Urls" << std::endl;

    return 0;
}

int BlackLibrary::UpdateEntries()
{
    std::cout << "Update entry tables" << std::endl;

    return 0;
}

int BlackLibrary::CleanStaging()
{
    std::cout << "Clean staging tables by comparing against entry tables" << std::endl;

    return 0;
}

} // namespace black_library
