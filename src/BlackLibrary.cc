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

BlackLibrary::BlackLibrary(const std::string &db_url, bool init_db) :
    blacklibrarydb_(db_url, init_db),
    url_puller_(nullptr),
    parse_entries_(),
    pull_urls_(),
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

    int seconds_counter = 0;

    while (!done_)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        
        if (seconds_counter >= 15)
            seconds_counter = 0;

        if (seconds_counter == 0)
        {
            RunOnce();
        }

        ++seconds_counter;

        if (done_)
            break;

        std::this_thread::sleep_until(deadline);
    }

    return 0;
}

int BlackLibrary::RunOnce()
{
    pull_urls_.clear();
    parse_entries_.clear();

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

    // pull_urls_ = url_puller_->PullUrls();

    pull_urls_.emplace_back("foo");
    pull_urls_.emplace_back("https://www.fictionpress.com/s/2961893/1/Mother-of-Learning");

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    std::cout << "Comparing Urls with database" << std::endl;
    parse_entries_.clear();

    for (auto it = pull_urls_.begin(); it < pull_urls_.end(); ++it)
    {
        std::cout << "CompareUrls: " << *it << std::endl;
        black_library::core::db::DBEntry entry;

        if (blacklibrarydb_.DoesBlackEntryUrlExist(*it))
        {
            black_library::core::db::DBStringResult res = blacklibrarydb_.GetBlackEntryUUIDFromUrl(*it);
            std::string UUID = res.result;
            entry = blacklibrarydb_.ReadBlackEntry(UUID);
        }
        else if (blacklibrarydb_.DoesStagingEntryUrlExist(*it))
        {
            black_library::core::db::DBStringResult res = blacklibrarydb_.GetStagingEntryUUIDFromUrl(*it);
            std::string UUID = res.result;
            entry = blacklibrarydb_.ReadStagingEntry(UUID);
        }
        else
        {
            entry.UUID = GenerateUUID();
            entry.url = *it;
            entry.last_url = *it;
        }

        std::cout << "UUID: " << entry.UUID << " url: " << entry.last_url << std::endl;
        parse_entries_.emplace_back(entry);
    }

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    std::cout << "Update staging tables of database with " << parse_entries_.size() << " entries" << std::endl;

    for (auto it = parse_entries_.begin(); it != parse_entries_.end(); ++it)
    {
        blacklibrarydb_.CreateStagingEntry(*it);
    }

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

// TODO: pull this into an include file and make it truly a uuid
// https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
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
