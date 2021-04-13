/**
 * BlackLibrary.cc
 */

#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>

#include <BlackLibrary.h>
#include <WgetUrlPuller.h>

namespace black_library {

BlackLibrary::BlackLibrary(const std::string &db_url, bool init_db) :
    blacklibrary_parser_manager_(""),
    blacklibrary_db_(db_url, init_db),
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

    std::cout << "\nRunning Black Library application" << std::endl;

    if (!blacklibrary_db_.IsReady())
    {
        std::cout << "Error: Black Library stalled, database not initalized" << std::endl;
        return -1;
    }

    if (PullUrls())
    {
        std::cout << "Error: Pulling Urls failed" << std::endl;
        return -1;
    }

    if (VerifyUrls())
    {
        std::cout << "Error: Verifying Urls failed" << std::endl;
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

    pull_urls_ = url_puller_->PullUrls();

    return 0;
}

int BlackLibrary::VerifyUrls()
{
    std::cout << "Verifying Urls" << std::endl;

    // make sure they contain a url pattern on the protected list

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    std::cout << "Comparing Urls with database" << std::endl;
    parse_entries_.clear();

    for (auto & url : pull_urls_)
    {
        PrintTabbed("CompareUrls: " + url, 1);
        black_library::core::db::DBEntry entry;

        if (blacklibrary_db_.DoesBlackEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetBlackEntryUUIDFromUrl(url);
            std::string UUID = res.result;
            entry = blacklibrary_db_.ReadBlackEntry(UUID);
        }
        // TODO: determine how to handle case in which there has been an update since the last staging thing "staging entry is stale"
        else if (blacklibrary_db_.DoesStagingEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetStagingEntryUUIDFromUrl(url);
            std::string UUID = res.result;
            entry = blacklibrary_db_.ReadStagingEntry(UUID);
        }
        else
        {
            entry.UUID = GenerateUUID();
            entry.url = url;
            entry.last_url = url;
        }

        std::cout << "UUID: " << entry.UUID << " url: " << entry.last_url << std::endl;
        parse_entries_.emplace_back(entry);
    }

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    std::cout << "Update staging tables of database with " << parse_entries_.size() << " entries" << std::endl;

    for (auto & entry : parse_entries_)
    {
        if (blacklibrary_db_.DoesStagingEntryUUIDExist(entry.UUID))
            continue;
        blacklibrary_db_.CreateStagingEntry(entry);
    }

    return 0;
}

int BlackLibrary::ParseUrls()
{
    std::cout << "Add " << parse_entries_.size() << " entries to parser manager" << std::endl;

    for (auto & entry : parse_entries_)
    {
        blacklibrary_parser_manager_.AddUrl(entry.url);
    }

    return 0;
}

int BlackLibrary::UpdateEntries()
{
    std::cout << "Update entry tables" << std::endl;

    UpdateBlackEntries();
    CleanStaging();

    return 0;
}

int BlackLibrary::UpdateBlackEntries()
{
    std::cout << "Update black entry table with successful parses" << std::endl;

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

int BlackLibrary::PrintTabbed(const std::string &statement, size_t num_tabs)
{
    std::stringstream ss;

    for (size_t i = 0; i < num_tabs; ++i)
    {
        ss << "\t";
    }

    ss << statement;

    std::cout << ss.str() << std::endl;

    return 0;
}

} // namespace black_library
