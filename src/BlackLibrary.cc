/**
 * BlackLibrary.cc
 */

#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>

#include <BlackLibrary.hh>
#include <WgetUrlPuller.hh>

namespace black_library {

BlackLibrary::BlackLibrary(const std::string &db_url) :
    blacklibrarydb_(db_url, true),
    url_puller_(nullptr),
    parse_urls_(),
    urls_(),
    rd_(),
    gen_(),
    dist0_(0, 15),
    dist1_(8, 11),
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

    if (CompareAndUpdateUrls())
    {
        std::cout << "Error Comparing and Updating Urls failed" << std::endl;
        return -1;
    }

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

    gen_ = std::mt19937_64(rd_());

    url_puller_ = std::make_shared<WgetUrlPuller>();

    return 0;
}

int BlackLibrary::PullUrls()
{
    std::cout << "Pulling Urls from source" << std::endl;

    urls_ = url_puller_->PullUrls();

    urls_.emplace_back("foo");

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    std::string UUID;
    std::string update_url;

    std::cout << "Comparing Urls with database" << std::endl;
    parse_urls_.clear();

    for (auto it = urls_.begin(); it < urls_.end(); ++it)
    {
        // auto i = std::distance(urls_.begin(), it);
        std::cout << "CompareUrls: " << *it << std::endl;

        if(blacklibrarydb_.DoesBlackEntryUrlExist(*it))
        {
            UUID = blacklibrarydb_.GetUUIDFromUrl(*it);
            update_url = blacklibrarydb_.GetUrlFromUUID(UUID);
        }
        else
        {
            UUID = GenerateUUID();
            update_url = *it;
        }

        std::cout << "UUID: " << UUID << " url: " << update_url << std::endl;
        parse_urls_.emplace(UUID, update_url);
    }

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    std::cout << "Update staging tables of database" << std::endl;

    // add everything in parse_urls_ to staging entries

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

std::string BlackLibrary::GenerateUUID()
{
    std::stringstream ss;
    size_t i;

    ss << std::hex;
    for (i = 0; i < 8; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    for (i = 0; i < 4; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-4";
    for (i = 0; i < 3; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    ss << dist1_(gen_);
    for (i = 0; i < 3; ++i)
    {
        ss << dist0_(gen_);
    }
    ss << "-";
    for (i = 0; i < 12; ++i)
    {
        ss << dist0_(gen_);
    }

    return ss.str();
}

} // namespace black_library
