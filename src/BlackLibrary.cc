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
    logger_name_("black_library"),
    database_parser_mutex_(),
    debug_target_(false),
    done_(true)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

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

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);

    BlackLibraryCommon::LogInfo(logger_name_, "Initializing BlackLibrary application");

    gen_ = std::mt19937_64(rd_());

    url_puller_ = std::make_shared<WgetUrlPuller>();

    parser_manager_.RegisterDatabaseStatusCallback(
        [&](BlackLibraryParsers::ParserJobResult result)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!result.is_error_job)
            {
                if (!blacklibrary_db_.DoesWorkEntryUUIDExist(result.metadata.uuid))
                {
                    BlackLibraryCommon::LogError(logger_name_, "Status entry with UUID: {} does not exist", result.metadata.uuid);
                    return;
                }

                auto work_entry = blacklibrary_db_.ReadWorkEntry(result.metadata.uuid);

                if (UpdateDatabaseWithResult(work_entry, result))
                {
                    BlackLibraryCommon::LogError(logger_name_, "Failed to update database with result UUID: {}", result.metadata.uuid);
                }

                return;
            }
            else
            {
                if (!blacklibrary_db_.DoesErrorEntryExist(result.metadata.uuid, result.start_number))
                {
                    BlackLibraryCommon::LogError(logger_name_, "Error entry with UUID: {} does not exist", result.metadata.uuid);
                    return;
                }

                if (blacklibrary_db_.DeleteErrorEntry(result.metadata.uuid, result.start_number))
                {
                    BlackLibraryCommon::LogError(logger_name_, "Failed to delete error entry with UUID: {} and progress number: {}", result.metadata.uuid, result.start_number);
                    return;
                }
            }
        }
    );
    parser_manager_.RegisterMd5CheckCallback(
        [&](const std::string &md5_sum, const std::string &uuid)
        {
            BlackLibraryCommon::LogDebug(logger_name_, "foo");
            BlackLibraryCommon::Md5Sum md5;
            if (!blacklibrary_db_.DoesWorkEntryUUIDExist(uuid))
            {
                BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
                return md5;
            }

            md5 = blacklibrary_db_.GetMd5SumFromMd5Sum(md5_sum, uuid);

            return md5;
        }
    );
    parser_manager_.RegisterMd5ReadCallback(
        [&](const std::string &uuid, const std::string &url)
        {
            BlackLibraryCommon::Md5Sum md5;
            if (!blacklibrary_db_.DoesWorkEntryUUIDExist(uuid))
            {
                BlackLibraryCommon::LogWarn(logger_name_, "Work entry with UUID: {} does not exist for md5 read", uuid);
                return md5;
            }
            if (!blacklibrary_db_.DoesMd5SumExistUrl(uuid, url))
            {
                BlackLibraryCommon::LogDebug(logger_name_, "Read md5 UUID: {} url: {} failed md5 sum does not exist", uuid, url);
                return md5;
            }

            md5 = blacklibrary_db_.ReadMd5SumUrl(uuid, url);

            return md5;
        }
    );
    parser_manager_.RegisterMd5sReadCallback(
        [&](const std::string &uuid)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);

            return blacklibrary_db_.GetMd5SumsFromUUID(uuid);
        }
    );
    parser_manager_.RegisterMd5UpdateCallback(
        [&](const std::string &uuid, size_t index_num, const std::string &md5_sum, time_t date, const std::string &url, uint64_t version_num)
        {
            BlackLibraryCommon::Md5Sum md5 = { uuid, index_num, md5_sum, date, url, version_num };

            // if exact copy already exists print warning, previous step should have already caught
            // if (blacklibrary_db_.DoesMd5SumExistExact())
            // {
            //     BlackLibraryCommon::LogError(logger_name_, "Exact copy of md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} already exists", uuid, index_num, md5_sum, date, url);
            //     return;
            // }

            if (blacklibrary_db_.DoesMd5SumExistIndexNum(uuid, index_num))
            {
                if (blacklibrary_db_.UpdateMd5Sum(md5))
                {
                    BlackLibraryCommon::LogError(logger_name_, "Update md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} already exists", uuid, index_num, md5_sum, date, url, version_num);
                    return;
                }
                
                return;
            }

            // otherwise, create a new one
            if (blacklibrary_db_.CreateMd5Sum(md5))
            {
                BlackLibraryCommon::LogError(logger_name_, "Create md5 UUID: {} index_num: {} md5_sum: {} date: {} url: {} failed", uuid, index_num, md5_sum, date, url, version_num);
                return;
            }
        }
    );
    parser_manager_.RegisterProgressNumberCallback(
        [&](const std::string &uuid, size_t progress_num, bool error)
        {
            const std::lock_guard<std::mutex> lock(database_parser_mutex_);
            if (!blacklibrary_db_.DoesWorkEntryUUIDExist(uuid))
            {
                BlackLibraryCommon::LogWarn(logger_name_, "Progress number callback entry with UUID: {} does not exist", uuid);
                return;
            }

            if (error)
            {
                if (blacklibrary_db_.DoesErrorEntryExist(uuid, progress_num))
                {
                    BlackLibraryCommon::LogWarn(logger_name_, "Error entry with UUID: {} progress_num: {} already exists", uuid, progress_num);
                    return;
                }

                BlackLibraryDB::DBErrorEntry entry = { uuid, progress_num };

                blacklibrary_db_.CreateErrorEntry(entry);
            }

            auto work_entry = blacklibrary_db_.ReadWorkEntry(uuid);

            if (work_entry.series_length < progress_num)
                work_entry.series_length = progress_num;

            if (blacklibrary_db_.UpdateWorkEntry(work_entry))
            {
                BlackLibraryCommon::LogError(logger_name_, "Work entry with UUID: {} failed to be updated", work_entry.uuid);
                return;
            }
        }
    );
    parser_manager_.RegisterVersionReadNumCallback(
        [&](const std::string &uuid, size_t index_num)
        {
            uint16_t version_num = 0;
            if (!blacklibrary_db_.DoesWorkEntryUUIDExist(uuid))
            {
                BlackLibraryCommon::LogWarn(logger_name_, "Register version read num entry with UUID: {} does not exist", uuid);
                return version_num;
            }
            if (!blacklibrary_db_.DoesMd5SumExistIndexNum(uuid, index_num))
            {
                BlackLibraryCommon::LogDebug(logger_name_, "Read version num with UUID: {} index_num: {} failed, MD5 sum does not exist", uuid, index_num);
                return version_num;
            }

            version_num = blacklibrary_db_.GetVersionFromMd5(uuid, index_num);

            return version_num;
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
    // use system_clock instead of steady_clock for systems that suspend on inactivity
    auto now_time = std::chrono::system_clock::now();
    auto run_deadline = now_time;

    while (!done_)
    {
        now_time = std::chrono::system_clock::now();
        const auto deadline = now_time + std::chrono::milliseconds(1000);

        if ((run_deadline - now_time).count() < 0)
        {
            uint8_t retries = 5;

            while (retries > 0)
            {
                if (!RunOnce())
                    break;

                std::this_thread::sleep_for(std::chrono::seconds(1));

                --retries;

                BlackLibraryCommon::LogWarn(logger_name_, "Failed run once, {} retries remaining", retries);
            }

            run_deadline = now_time + std::chrono::seconds(BLACKLIBRARY_FREQUENCY);
            auto info_time = std::chrono::system_clock::now() + std::chrono::seconds(BLACKLIBRARY_FREQUENCY);
            const std::string iso_info_time = BlackLibraryCommon::GetISOTimeString(std::chrono::system_clock::to_time_t(info_time));
            BlackLibraryCommon::LogInfo(logger_name_, "Next scheduled run at {}", iso_info_time);
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

    BlackLibraryCommon::LogInfo(logger_name_, "Running Black Library application");

    if (!blacklibrary_db_.IsReady())
    {
        BlackLibraryCommon::LogError(logger_name_, "Black Library stalled, database not initalized/ready");
        return -1;
    }

    if (!parser_manager_.IsReady())
    {
        BlackLibraryCommon::LogError(logger_name_, "Black Library stalled, parser manager not initalized/ready");
        return -1;
    }

    if (PullUrls())
    {
        BlackLibraryCommon::LogError(logger_name_, "Pulling Urls failed");
        return -1;
    }

    if (VerifyUrls())
    {
        BlackLibraryCommon::LogError(logger_name_, "Verifying Urls failed");
        return -1;
    }

    if (CompareAndUpdateUrls())
    {
        BlackLibraryCommon::LogError(logger_name_, "Comparing and Updating Urls failed");
        return -1;
    }

    ParseUrls();

    ParseErrorEntries();

    return 0;
}

int BlackLibrary::Stop()
{
    done_ = true;

    BlackLibraryCommon::LogWarn(logger_name_, "Joining manager thread");

    curl_global_cleanup();

    if (manager_thread_.joinable())
        manager_thread_.join();

    BlackLibraryCommon::LogInfo(logger_name_, "Stopping BlackLibrary");

    // clean up memory allocated by xml
    xmlCleanupParser();

    // shutdown logging manually just in case termination flush fails
    spdlog::shutdown();

    return 0;
}

int BlackLibrary::PullUrls()
{
    BlackLibraryCommon::LogInfo(logger_name_, "Pulling Urls from source");

    // puller sanatizes urls
    pull_urls_ = url_puller_->PullUrls(debug_target_);

    BlackLibraryCommon::LogInfo(logger_name_, "Pulled {} urls", pull_urls_.size());

    if (pull_urls_.size() <= 0)
        return -1;

    return 0;
}

int BlackLibrary::VerifyUrls()
{
    BlackLibraryCommon::LogInfo(logger_name_, "Verifying Urls");

    // remove if url not on the protected list
    pull_urls_.erase(std::remove_if(pull_urls_.begin(), pull_urls_.end(), [](const std::string &str){ return !BlackLibraryCommon::IsSourceInformationMember(str); }), pull_urls_.end());

    BlackLibraryCommon::LogInfo(logger_name_, "After source information check {} urls", pull_urls_.size());

    // remove duplicate urls, sorting is faster then using a set for low number of duplicates
    std::sort(pull_urls_.begin(), pull_urls_.end());
    pull_urls_.erase(std::unique(pull_urls_.begin(), pull_urls_.end()), pull_urls_.end());

    BlackLibraryCommon::LogInfo(logger_name_, "Verified {} urls", pull_urls_.size());

    return 0;
}

int BlackLibrary::CompareAndUpdateUrls()
{
    BlackLibraryCommon::LogInfo(logger_name_, "Comparing Urls with database");

    parse_entries_.clear();

    for (const auto & url : pull_urls_)
    {
        BlackLibraryDB::DBEntry entry;
        std::string type = "unknown";

        // check entries to continue jobs that were still in progress
        if (blacklibrary_db_.DoesWorkEntryUrlExist(url))
        {
            auto res = blacklibrary_db_.GetWorkEntryUUIDFromUrl(url);
            std::string uuid = res.result;
            entry = blacklibrary_db_.ReadWorkEntry(uuid);
            entry.check_date = BlackLibraryCommon::GetUnixTime();
            entry.processing = true;
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
            // add to db
            blacklibrary_db_.CreateWorkEntry(entry);
        }

        BlackLibraryCommon::LogDebug(logger_name_, "Type: {} UUID: {} last_url: {} length: {}", type, entry.uuid, entry.last_url, entry.series_length);

        parse_entries_.emplace_back(entry);
    }

    return 0;
}

int BlackLibrary::ParseUrls()
{
    BlackLibraryCommon::LogInfo(logger_name_, "Adding {} jobs to parser manager", parse_entries_.size());

    for (auto & entry : parse_entries_)
    {
        parser_manager_.AddJob(entry.uuid, entry.url, entry.last_url, entry.series_length);
    }

    return 0;
}

int BlackLibrary::ParseErrorEntries()
{
    auto error_list = blacklibrary_db_.GetErrorEntryList();

    BlackLibraryCommon::LogInfo(logger_name_, "Adding {} error jobs to parser manager", error_list.size());

    for (const auto & error : error_list)
    {
        if (!blacklibrary_db_.DoesWorkEntryUUIDExist(error.uuid))
        {
            BlackLibraryCommon::LogWarn(logger_name_, "Work entry does not exist for error UUID: {}", error.uuid);
            continue;
        }

        auto res = blacklibrary_db_.GetWorkEntryUrlFromUUID(error.uuid);

        if (res.error)
        {
            BlackLibraryCommon::LogWarn(logger_name_, "Failed to get work entry from {}", error);
            continue;
        }
        
        std::string url = res.result;
        BlackLibraryDB::DBEntry entry = blacklibrary_db_.ReadWorkEntry(error.uuid);

        if (entry.uuid.empty())
        {
            BlackLibraryCommon::LogWarn(logger_name_, "Failed to read entry {}", entry);
            continue;
        }

        parser_manager_.AddJob(error.uuid, url, url, error.progress_num, error.progress_num, true);
    }

    return 0;
}

int BlackLibrary::UpdateDatabaseWithResult(BlackLibraryDB::DBEntry &entry, const BlackLibraryParsers::ParserJobResult &result)
{
    // entry.title = result.metadata.title;
    // entry.nickname = result.metadata.nickname;
    // entry.source = result.metadata.source;
    // entry.series = result.metadata.series;
    // entry.author = result.metadata.author;
    BlackLibraryCommon::LogDebug(logger_name_, "Update database with {}", result);
    if (!result.metadata.title.empty())
        entry.title = result.metadata.title;
    if (!result.metadata.nickname.empty())
        entry.nickname = result.metadata.nickname;
    if (!result.metadata.source.empty())
        entry.source = result.metadata.source;
    if (!result.metadata.series.empty())
        entry.series = result.metadata.series;
    if (!result.metadata.author.empty())
        entry.author = result.metadata.author;

    if (!result.metadata.last_url.empty())
        entry.last_url = result.metadata.last_url;
    else
        entry.last_url = entry.url;

    if (entry.update_date < result.metadata.update_date)
        entry.update_date = result.metadata.update_date;

    if (entry.series_length < result.metadata.series_length)
        entry.series_length = result.metadata.series_length;

    if (!result.metadata.media_path.empty())
        entry.media_path = result.metadata.media_path;

    // update work entry
    if (!blacklibrary_db_.DoesWorkEntryUUIDExist(result.metadata.uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update work entry UUID: {} does not exist", result.metadata.uuid);
        return -1;
    }

    // Do not set processing off if still working on uuid
    if (parser_manager_.StillWorkingOn(result.metadata.uuid))
    {
        BlackLibraryCommon::LogWarn(logger_name_, "Still working on job with UUID: {}", result.metadata.uuid);
        entry.processing = true;
    }
    else
    {
        BlackLibraryCommon::LogDebug(logger_name_, "Finished processing entry with UUID: {}", entry.uuid);
        entry.processing = false;
    }

    if (blacklibrary_db_.UpdateWorkEntry(entry))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update work entry, UUID: {}", entry.uuid);
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
