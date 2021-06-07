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

BlackLibrary::BlackLibrary(const std::string &db_path, const std::string &storage_path, bool init_db) :
    parser_manager_(storage_path, ""),
    blacklibrary_db_(db_path, init_db),
    url_puller_(nullptr),
    parse_entries_(),
    pull_urls_(),
    manager_thread_(),
    rd_(),
    gen_(),
    dist0_(0, 15),
    dist1_(8, 11),
    database_parser_mutex_(),
    done_(true)
{
    Init();
}

int BlackLibrary::Run()
{
    int seconds_counter = 0;

    while (!done_)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);

        if (seconds_counter >= 21600)
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
        std::cout << "Error: Black Library stalled, database not initalized/ready" << std::endl;
        return -1;
    }

    if (!parser_manager_.IsReady())
    {
        std::cout << "Error: Black Library stalled, parser manager not initalized/ready" << std::endl;
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
        std::cout << "Error: Comparing and Updating Urls failed" << std::endl;
        return -1;
    }


    if (UpdateStaging())
    {
        std::cout << "Error: Updating staging table failed" << std::endl;
        return -1;
    }

    ParseUrls();

    return 0;
}

int BlackLibrary::Stop()
{
    done_ = true;

    std::cout << "Joining manager thread" << std::endl;

    curl_global_cleanup();

    if (manager_thread_.joinable())
        manager_thread_.join();

    std::cout << "Stopping BlackLibrary" << std::endl;

    return 0;
}

int BlackLibrary::Init()
{
    std::cout << "Initializing BlackLibrary" << std::endl;

    gen_ = std::mt19937_64(rd_());

    url_puller_ = std::make_shared<WgetUrlPuller>();

    parser_manager_.RegisterProgressNumberCallback(
        [this](const std::string &uuid, size_t progress_num, bool error)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!blacklibrary_db_.DoesStagingEntryUUIDExist(uuid))
            {
                std::cout << "Error: Staging entry with UUID: " << uuid << " does not exist" << std::endl;
                return;
            }

            if (error)
            {
                if (blacklibrary_db_.DoesErrorEntryExist(uuid, progress_num))
                {
                    std::cout << "Error: Error entry with UUID: " << uuid << " progress_num: " << progress_num << " already exists" << std::endl;
                    return;
                }

                black_library::core::db::ErrorEntry entry = { uuid, progress_num };

                blacklibrary_db_.CreateErrorEntry(entry);
            }

            auto staging_entry = blacklibrary_db_.ReadStagingEntry(uuid);

            staging_entry.series_length = progress_num;

            if (blacklibrary_db_.UpdateStagingEntry(staging_entry))
            {
                std::cout << "Error: Staging entry with UUID: " << staging_entry.uuid << " could not be updated" << std::endl;
                return;
            }
        }
    );
    parser_manager_.RegisterDatabaseStatusCallback(
        [this](core::parsers::ParserJobResult result)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!blacklibrary_db_.DoesStagingEntryUUIDExist(result.metadata.uuid))
            {
                std::cout << "Error: Staging entry with UUID: " << result.metadata.uuid << " does not exist" << std::endl;
                return;
            }

            auto staging_entry = blacklibrary_db_.ReadStagingEntry(result.metadata.uuid);

            if (UpdateDatabaseWithResult(staging_entry, result))
            {
                std::cout << "Error: could not update database with result UUID: " << result.metadata.uuid << std::endl;
            }

        }
    );

    done_ = false;

    manager_thread_ = std::thread([this](){
        while (!done_ && parser_manager_.GetDone())
        {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);

            if (done_)
                break;

            parser_manager_.RunOnce();

            std::this_thread::sleep_until(deadline);
        }

        parser_manager_.Stop();
    });

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

    // remove duplicate urls, sorting is faster then using a set for low number of duplicates
    std::sort(pull_urls_.begin(), pull_urls_.end());
    pull_urls_.erase(std::unique(pull_urls_.begin(), pull_urls_.end()), pull_urls_.end());

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    std::cout << "Comparing Urls with database" << std::endl << std::endl;;
    parse_entries_.clear();

    for (auto & url : pull_urls_)
    {
        black_library::core::db::DBEntry entry;

        // check staging entries first to continue jobs that were still in progress
        if (blacklibrary_db_.DoesStagingEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetStagingEntryUUIDFromUrl(url);
            std::string uuid = res.result;
            entry = blacklibrary_db_.ReadStagingEntry(uuid);
        }
        else if (blacklibrary_db_.DoesBlackEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetBlackEntryUUIDFromUrl(url);
            std::string uuid = res.result;
            entry = blacklibrary_db_.ReadBlackEntry(uuid);
        }
        else
        {
            entry.uuid = GenerateUUID();
            entry.url = url;
            entry.last_url = url;
            entry.series_length = 1;
        }

        std::cout << "UUID: " << entry.uuid << " last_url: " << entry.last_url << " length: " << entry.series_length << std::endl << std::endl;

        parse_entries_.emplace_back(entry);
    }

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    std::cout << "Update staging table of database with " << parse_entries_.size() << " entries" << std::endl << std::endl;

    size_t num_new_entries = 0;
    size_t num_existing_entries = 0;

    for (auto & entry : parse_entries_)
    {
        if (blacklibrary_db_.DoesStagingEntryUUIDExist(entry.uuid))
        {
            ++num_existing_entries;
            continue;
        }
        else
        {
            blacklibrary_db_.CreateStagingEntry(entry);
            ++num_new_entries;
        }
    }

    std::cout << "Staging table added " << num_new_entries << " entries and has " << num_existing_entries << " existing entries" << std::endl;

    return 0;
}

int BlackLibrary::ParseUrls()
{
    std::cout << "Adding " << parse_entries_.size() << " jobs to parser manager" << std::endl << std::endl;

    for (auto & entry : parse_entries_)
    {
        parser_manager_.AddJob(entry.uuid, entry.url, entry.series_length);
    }

    return 0;
}

int BlackLibrary::UpdateDatabaseWithResult(core::db::DBEntry &entry, const core::parsers::ParserJobResult &result)
{
    entry.last_url = result.metadata.last_url;
    entry.series_length = result.metadata.series_length;
    entry.update_date = result.metadata.update_date;
    entry.title = result.metadata.title;
    entry.author = result.metadata.author;
    entry.nickname = result.metadata.nickname;
    entry.source = result.metadata.source;
    entry.series = result.metadata.series;

    // if entry already exists, just update, else create new
    if (blacklibrary_db_.DoesBlackEntryUUIDExist(result.metadata.uuid))
    {
        int res = blacklibrary_db_.UpdateBlackEntry(entry);

        if (res)
            return -1;
    }
    else
    {
        entry.media_path = result.metadata.media_path;
        entry.birth_date = result.metadata.update_date;
        
        int res = blacklibrary_db_.CreateBlackEntry(entry);

        if (res)
            return -1;
    }

    if (blacklibrary_db_.DeleteStagingEntry(result.metadata.uuid))
    {
        std::cout << "Error: could not delete staging entry with UUID: " << result.metadata.uuid << std::endl;
        return -1;
    }

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
