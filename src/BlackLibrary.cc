/**
 * BlackLibrary.cc
 */

#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>

#include <LogOperations.h>
#include <TimeOperations.h>
#include <VersionOperations.h>

#include <BlackLibrary.h>
#include <WgetUrlPuller.h>

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;
namespace BlackLibraryParsers = black_library::core::parsers;

const int BLACKLIBRARY_FREQUENCY = 24*60*60;

BlackLibrary::BlackLibrary(const njson &config) :
    parser_manager_(config),
    blacklibrary_db_(config),
    url_puller_(nullptr),
    parse_entries_(),
    pull_urls_(),
    manager_thread_(),
    rd_(),
    gen_(),
    dist0_(0, 15),
    dist1_(8, 11),
    database_parser_mutex_(),
    debug_target_(false),
    done_(true)
{
    njson nconfig = config["config"];

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    bool logger_level = BlackLibraryCommon::DefaultLogLevel;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    if (nconfig.contains("main_app_debug_log"))
    {
        logger_level = nconfig["main_app_debug_log"];
    }

    if (nconfig.contains("url_puller_debug"))
    {
        debug_target_ = nconfig["url_puller_debug"];
    }

    BlackLibraryCommon::InitRotatingLogger("black_library", logger_path, logger_level);

    BlackLibraryCommon::LogInfo("black_library", "Initializing BlackLibrary application");

    gen_ = std::mt19937_64(rd_());

    url_puller_ = std::make_shared<WgetUrlPuller>();

    parser_manager_.RegisterDatabaseStatusCallback(
        [&](BlackLibraryParsers::ParserJobResult result)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!result.is_error_job)
            {
                if (!blacklibrary_db_.DoesStagingEntryUUIDExist(result.metadata.uuid))
                {
                    BlackLibraryCommon::LogError("black_library", "Status staging entry with UUID: {} does not exist", result.metadata.uuid);
                    return;
                }

                auto staging_entry = blacklibrary_db_.ReadStagingEntry(result.metadata.uuid);

                if (UpdateDatabaseWithResult(staging_entry, result))
                {
                    BlackLibraryCommon::LogError("black_library", "Failed to update database with result with UUID: {}", result.metadata.uuid);
                }

                return;
            }

            if (!blacklibrary_db_.DoesErrorEntryExist(result.metadata.uuid, result.start_number))
            {
                BlackLibraryCommon::LogError("black_library", "Error entry with UUID: {} does not exist", result.metadata.uuid);
                return;
            }

            if (blacklibrary_db_.DeleteErrorEntry(result.metadata.uuid, result.start_number))
            {
                BlackLibraryCommon::LogError("black_library", "Failed to delete error entry with UUID: {} and progress number: {}", result.metadata.uuid, result.start_number);
                return;
            }

        }
    );
    parser_manager_.RegisterProgressNumberCallback(
        [&](const std::string &uuid, size_t progress_num, bool error)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!blacklibrary_db_.DoesStagingEntryUUIDExist(uuid))
            {
                BlackLibraryCommon::LogWarn("black_library", "Progress number staging entry with UUID: {} does not exist", uuid);
                return;
            }

            if (error)
            {
                if (blacklibrary_db_.DoesErrorEntryExist(uuid, progress_num))
                {
                    BlackLibraryCommon::LogWarn("black_library", "Error entry with UUID: {} progress_num: {}", uuid, progress_num);
                    return;
                }

                BlackLibraryDB::DBErrorEntry entry = { uuid, progress_num };

                blacklibrary_db_.CreateErrorEntry(entry);
            }

            auto staging_entry = blacklibrary_db_.ReadStagingEntry(uuid);

            if (staging_entry.series_length < progress_num)
                staging_entry.series_length = progress_num;

            if (blacklibrary_db_.UpdateStagingEntry(staging_entry))
            {
                BlackLibraryCommon::LogError("black_library", "Staging entry with UUID: {} failed to be updated", staging_entry.uuid);
                return;
            }
        }
    );
    parser_manager_.RegisterVersionReadCallback(
        [&](const std::string &uuid, size_t index_num)
        {
            std::string checksum = BlackLibraryCommon::EmptyMD5Version;
            if (!blacklibrary_db_.DoesMd5SumExist(uuid, index_num))
            {
                BlackLibraryCommon::LogDebug("black_library", "Read version UUID: {} index_num: {} failed MD5 sum does not exist", uuid, index_num);
                return checksum;
            }

            auto version = blacklibrary_db_.ReadMd5Sum(uuid, index_num);
            checksum = version.md5_sum;

            return checksum;
        }
    );
    parser_manager_.RegisterVersionReadNumCallback(
        [&](const std::string &uuid, size_t index_num)
        {
            uint16_t version_num = 0;
            if (!blacklibrary_db_.DoesMd5SumExist(uuid, index_num))
            {
                BlackLibraryCommon::LogDebug("black_library", "Read version num with UUID: {} index_num: {} failed, MD5 sum does not exist", uuid, index_num);
                return version_num;
            }

            version_num = blacklibrary_db_.GetVersionFromMd5(uuid, index_num);

            return version_num;
        }
    );
    parser_manager_.RegisterVersionUpdateCallback(
        [&](const std::string &uuid, size_t index_num, const std::string &md5_sum, uint64_t version_num)
        {
            BlackLibraryDB::DBMd5Sum md5 = { uuid, index_num, md5_sum, version_num };

            if (!blacklibrary_db_.DoesMd5SumExist(uuid, index_num))
            {
                if (blacklibrary_db_.CreateMd5Sum(md5))
                {
                    BlackLibraryCommon::LogError("black_library", "Create version UUID: {} index_num: {} md5_sum: {} failed", uuid, index_num, md5_sum);
                    return;
                }
            }

            if (blacklibrary_db_.UpdateMd5Sum(md5))
            {
                BlackLibraryCommon::LogError("black_library", "Update version UUID: {} index_num: {} md5_sum: {} failed", uuid, index_num, md5_sum);
                return;
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
}

int BlackLibrary::Run()
{
    auto now_time = std::chrono::steady_clock::now();
    auto run_deadline = now_time;

    while (!done_)
    {
        now_time = std::chrono::steady_clock::now();
        const auto deadline = now_time + std::chrono::milliseconds(1000);

        if ((run_deadline - now_time).count() < 0)
        {
            RunOnce();
            run_deadline = now_time + std::chrono::seconds(BLACKLIBRARY_FREQUENCY);
            auto info_time = std::chrono::system_clock::now() + std::chrono::seconds(BLACKLIBRARY_FREQUENCY);
            const std::string iso_info_time = BlackLibraryCommon::GetISOTimeString(std::chrono::system_clock::to_time_t(info_time));
            BlackLibraryCommon::LogInfo("black_library", "Next scheduled run at {}", iso_info_time);
        }

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

    BlackLibraryCommon::LogInfo("black_library", "Running Black Library application");

    if (!blacklibrary_db_.IsReady())
    {
        BlackLibraryCommon::LogError("black_library", "Black Library stalled, database not initalized/ready");
        return -1;
    }

    if (!parser_manager_.IsReady())
    {
        BlackLibraryCommon::LogError("black_library", "Black Library stalled, parser manager not initalized/ready");
        return -1;
    }

    if (PullUrls())
    {
        BlackLibraryCommon::LogError("black_library", "Pulling Urls failed");
        return -1;
    }

    if (VerifyUrls())
    {
        BlackLibraryCommon::LogError("black_library", "Verifying Urls failed");
        return -1;
    }

    if (CompareAndUpdateUrls())
    {
        BlackLibraryCommon::LogError("black_library", "Comparing and Updating Urls failed");
        return -1;
    }


    if (UpdateStaging())
    {
        BlackLibraryCommon::LogError("black_library", "Updating staging table failed");
        return -1;
    }

    ParseUrls();

    ParserErrorEntries();

    return 0;
}

int BlackLibrary::Stop()
{
    done_ = true;

    BlackLibraryCommon::LogWarn("black_library", "Joining manager thread");

    curl_global_cleanup();

    if (manager_thread_.joinable())
        manager_thread_.join();

    BlackLibraryCommon::LogInfo("black_library", "Stopping BlackLibrary");

    // shutdown logging manually just in case termination flush fails
    spdlog::shutdown();

    return 0;
}

int BlackLibrary::PullUrls()
{
    BlackLibraryCommon::LogInfo("black_library", "Pulling Urls from source");

    // puller sanatizes urls
    pull_urls_ = url_puller_->PullUrls(debug_target_);

    BlackLibraryCommon::LogInfo("black_library", "Pulled {} urls", pull_urls_.size());

    if (pull_urls_.size() <= 0)
        return -1;

    return 0;
}

int BlackLibrary::VerifyUrls()
{
    BlackLibraryCommon::LogInfo("black_library", "Verifying Urls");

    // make sure they contain a url pattern on the protected list
    pull_urls_.erase(std::remove_if(pull_urls_.begin(), pull_urls_.end(), std::not1(BlackLibraryCommon::SourceInformationMember())), pull_urls_.end());

    BlackLibraryCommon::LogInfo("black_library", "After source information check {} urls", pull_urls_.size());

    // remove duplicate urls, sorting is faster then using a set for low number of duplicates
    std::sort(pull_urls_.begin(), pull_urls_.end());
    pull_urls_.erase(std::unique(pull_urls_.begin(), pull_urls_.end()), pull_urls_.end());

    BlackLibraryCommon::LogInfo("black_library", "Verified {} urls", pull_urls_.size());

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    BlackLibraryCommon::LogInfo("black_library", "Comparing Urls with database");

    parse_entries_.clear();

    for (const auto & url : pull_urls_)
    {
        BlackLibraryDB::DBEntry entry;
        std::string type = "unknown";

        // check staging entries first to continue jobs that were still in progress
        if (blacklibrary_db_.DoesStagingEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetStagingEntryUUIDFromUrl(url);
            std::string uuid = res.result;
            entry = blacklibrary_db_.ReadStagingEntry(uuid);
            type = "staging";
        }
        else if (blacklibrary_db_.DoesBlackEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetBlackEntryUUIDFromUrl(url);
            std::string uuid = res.result;
            entry = blacklibrary_db_.ReadBlackEntry(uuid);
            entry.check_date = BlackLibraryCommon::GetUnixTime();
            type = "black";
        }
        else
        {
            entry.uuid = GenerateUUID();
            entry.url = url;
            entry.last_url = url;
            entry.series_length = 1;
            entry.birth_date = BlackLibraryCommon::GetUnixTime();
            entry.check_date = BlackLibraryCommon::GetUnixTime();
            type = "new";
        }

        BlackLibraryCommon::LogDebug("black_library", "Type: {} UUID: {} last_url: {} length: {}", type, entry.uuid, entry.last_url, entry.series_length);

        parse_entries_.emplace_back(entry);
    }

    return 0;
}

int BlackLibrary::UpdateStaging()
{
    BlackLibraryCommon::LogInfo("black_library", "Update staging table of database with {} entries", parse_entries_.size());

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

    BlackLibraryCommon::LogInfo("black_library", "Staging table added {} entries and has {} existing entries", num_new_entries, num_existing_entries);

    return 0;
}

int BlackLibrary::ParseUrls()
{
    BlackLibraryCommon::LogInfo("black_library", "Adding {} jobs to parser manager", parse_entries_.size());

    for (auto & entry : parse_entries_)
    {
        parser_manager_.AddJob(entry.uuid, entry.url, entry.last_url, entry.series_length);
    }

    return 0;
}

int BlackLibrary::ParserErrorEntries()
{
    auto error_list = blacklibrary_db_.GetErrorEntryList();

    BlackLibraryCommon::LogInfo("black_library", "Adding {} error jobs to parser manager", error_list.size());

    for (const auto & error : error_list)
    {
        std::string url;

        if (blacklibrary_db_.DoesStagingEntryUUIDExist(error.uuid))
        {
            auto res = blacklibrary_db_.GetStagingEntryUrlFromUUID(error.uuid);

            if (res.error)
                continue;
            
            url = res.result;
        }
        else if (blacklibrary_db_.DoesBlackEntryUUIDExist(error.uuid))
        {
            auto res = blacklibrary_db_.GetStagingEntryUrlFromUUID(error.uuid);

            if (res.error)
                continue;
            
            url = res.result;
            BlackLibraryDB::DBEntry entry = blacklibrary_db_.ReadBlackEntry(error.uuid);
            if (entry.uuid.empty())
                continue;
            blacklibrary_db_.CreateStagingEntry(entry);
        }
        else
        {
            BlackLibraryCommon::LogWarn("black_library", "Failed to match error entry {}");
            continue;
        }

        parser_manager_.AddJob(error.uuid, url, url, error.progress_num, error.progress_num, true);
    }

    return 0;
}

int BlackLibrary::UpdateDatabaseWithResult(BlackLibraryDB::DBEntry &entry, const BlackLibraryParsers::ParserJobResult &result)
{
    // TODO staging should check against black for these
    BlackLibraryCommon::LogDebug("black_library", "Update database with {}", result);
    // if (entry.title.empty() && !result.metadata.title.empty())
    //     entry.title = result.metadata.title;
    // if (entry.nickname.empty() && !result.metadata.nickname.empty())
    //     entry.nickname = result.metadata.nickname;
    // if (entry.source.empty() && !result.metadata.source.empty())
    //     entry.source = result.metadata.source;
    // if (entry.series.empty() && !result.metadata.series.empty())
    //     entry.series = result.metadata.series;
    // if (entry.author.empty() && !result.metadata.author.empty())
    //     entry.author = result.metadata.author;

    if (!result.is_error_job)
    {
        entry.title = result.metadata.title;
        entry.nickname = result.metadata.nickname;
        entry.source = result.metadata.source;
        entry.series = result.metadata.series;
        entry.author = result.metadata.author;

        entry.last_url = result.metadata.last_url;
    }
    else
    {
        entry.title = result.metadata.title;
        entry.nickname = result.metadata.nickname;
        entry.source = result.metadata.source;
        entry.series = result.metadata.series;
        entry.author = result.metadata.author;
    }

    if (entry.update_date < result.metadata.update_date)
        entry.update_date = result.metadata.update_date;

    if (entry.series_length < result.metadata.series_length)
    {
        entry.series_length = result.metadata.series_length;
    }

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
        
        int res = blacklibrary_db_.CreateBlackEntry(entry);

        if (res)
            return -1;
    }

    // Do not delete staging entry if still working on uuid
    if (parser_manager_.StillWorkingOn(result.metadata.uuid))
    {
        BlackLibraryCommon::LogInfo("black_library", "Still working on job with UUID: {}", result.metadata.uuid);
        return 0;
    }

    if (blacklibrary_db_.DeleteStagingEntry(result.metadata.uuid))
    {
        BlackLibraryCommon::LogError("black_library", "Failed to delete staging entry with UUID: {}", result.metadata.uuid);
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
