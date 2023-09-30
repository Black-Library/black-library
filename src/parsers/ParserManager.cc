/**
 * ParserManager.cc
 */

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include <FileOperations.h>
#include <LogOperations.h>
#include <VersionOperations.h>

#include <ParserManager.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

ParserManager::ParserManager(const njson &config, const std::shared_ptr<ParserDbAdapter> &db_adapter) :
    worker_map_(),
    parser_factory_(std::make_shared<ParserFactory>(config, db_adapter)),
    current_jobs_(),
    job_queue_(),
    result_queue_(),
    database_status_callback_(),
    progress_number_callback_(),
    version_read_num_callback_(),
    config_(config),
    done_(true),
    initialized_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    std::string storage_path = BlackLibraryCommon::DefaultStoragePath;
    bool logger_level = BlackLibraryCommon::DefaultLogLevel;
    int worker_count = 1;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    if (nconfig.contains("manager_debug_log"))
    {
        logger_level = nconfig["manager_debug_log"];
    }

    if (nconfig.contains("storage_path"))
    {
        storage_path = nconfig["storage_path"];
    }

    if (nconfig.contains("manager_worker_count"))
    {
        worker_count = nconfig["manager_worker_count"];
    }

    BlackLibraryCommon::InitRotatingLogger("parser_manager", logger_path, logger_level);

    // okay to pop_back(), string isn't empty
    if (storage_path.back() == '/')
        storage_path.pop_back();

    if (storage_path.empty())
    {
        storage_path = BlackLibraryCommon::DefaultStoragePath;
        BlackLibraryCommon::LogDebug("parser_manager", "Empty storage path given, using default: {}", storage_path);
    }

    if (!BlackLibraryCommon::CheckFilePermission(storage_path))
    {
        BlackLibraryCommon::LogError("parser_manager", "Failed to access storage path: {}", storage_path);
        return;
    }

    BlackLibraryCommon::LogInfo("parser_manager", "Using storage path: {}", storage_path);
    BlackLibraryCommon::LogDebug("parser_manager", "workers will use {} parsers", worker_count);

    AddWorker(parser_t::AO3_PARSER, worker_count);
    // AddWorker(parser_t::RR_PARSER, worker_count);
    AddWorker(parser_t::SBF_PARSER, worker_count);
    AddWorker(parser_t::SVF_PARSER, worker_count);
    // AddWorker(parser_t::WP_PARSER, worker_count);

    for (auto & worker : worker_map_)
    {
        worker.second->RegisterJobStatusCallback(
            [this](const ParserJob &parser_job, job_status_t job_status)
            {
                if (!current_jobs_.find_and_replace(std::make_pair(parser_job.uuid, parser_job.is_error_job), job_status))
                    BlackLibraryCommon::LogError("parser_manager", "Failed to replace job status for job with UUID: {}", parser_job.uuid);
            }
        );
        worker.second->RegisterManagerNotifyCallback(
            [this](ParserJobResult result)
            {
                BlackLibraryCommon::LogDebug("parser_manager", "Recieved result: {}", result);
                result_queue_.push(result);
            }
        );
        worker.second->RegisterProgressNumberCallback(
            [this](const std::string &uuid, size_t progress_num, bool error)
            {
                if (progress_number_callback_)
                    progress_number_callback_(uuid, progress_num, error);
            }
        );
    }

    initialized_ = true;
}

int ParserManager::Run()
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

int ParserManager::RunOnce()
{
    while (!job_queue_.empty())
    {
        auto job = job_queue_.pop();
        auto parser_type = GetParserTypeByUrl(job.url);

        if (parser_type == parser_t::ERROR_PARSER)
        {
            BlackLibraryCommon::LogError("parser_manager", "Failed to match url {} to parser", job.url);
            continue;
        }

        auto worker = worker_map_.find(parser_type);

        if (worker == worker_map_.end())
        {
            BlackLibraryCommon::LogError("parser_manager", "Failed to find parser with rep: {}", GetParserName(parser_type));
            continue;
        }

        worker->second->AddJob(job);
    }

    for (auto & worker : worker_map_)
    {
        worker.second->RunOnce();
    }

    while (!result_queue_.empty())
    {
        auto job_result = result_queue_.pop();
        if (job_result.metadata.uuid.empty() || job_result.metadata.url.empty())
        {
            BlackLibraryCommon::LogWarn("parser_manager", "Got job result with emptry UUID or url");
            continue;
        }

        BlackLibraryCommon::LogInfo("parser_manager", "Finished job with UUID: {} url: {} author: {} is_error_job: {}", job_result.metadata.uuid, job_result.metadata.url, job_result.metadata.author, job_result.is_error_job);

        if (current_jobs_.count(std::make_pair(job_result.metadata.uuid, job_result.is_error_job)))
        {
            current_jobs_.erase(std::make_pair(job_result.metadata.uuid, job_result.is_error_job));
        }
        else
        {
            BlackLibraryCommon::LogError("parser_manager", "Failed to find current job with UUID: {} is_error_job: {}", job_result.metadata.uuid, job_result.is_error_job);
        }

        if (job_result.has_error)
            BlackLibraryCommon::LogError("parser_manager", "Job error with UUID: {}, url: {}", job_result.metadata.uuid, job_result.metadata.url);

        if (!job_result.has_error && database_status_callback_)
            database_status_callback_(job_result);
    }

    return 0;
}

int ParserManager::Stop()
{
    done_ = true;

    for (auto & worker : worker_map_)
    {
        worker.second->Stop();
    }

    BlackLibraryCommon::LogInfo("parser_manager", "Stopped manager");

    BlackLibraryCommon::CloseLogger("parser_manager");

    return 0;
}

bool ParserManager::IsReady()
{
    return initialized_;
}

int ParserManager::AddJob(const std::string &uuid, const std::string &url, const std::string &last_url)
{
    AddJob(uuid, url, last_url, 1, 0, false);

    return 0;
}

int ParserManager::AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number)
{
    AddJob(uuid, url, last_url, start_number, 0, false);

    return 0;
}

int ParserManager::AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number, const size_t &end_number)
{
    AddJob(uuid, url, last_url, start_number, end_number, false);

    return 0;
}

int ParserManager::AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number, const size_t &end_number, const error_job_rep &is_error_job)
{
    ParserJob parser_job;
    
    parser_job.uuid = uuid;
    parser_job.url = url;
    parser_job.last_url = last_url;
    parser_job.start_number = start_number;
    parser_job.end_number = end_number;
    parser_job.is_error_job = is_error_job;

    // parser_job.start_number should be at least 1
    if (parser_job.start_number <= 0)
    {
        parser_job.start_number = 1;
    }

    BlackLibraryCommon::LogDebug("parser_manager", "Adding job: {}", parser_job);

    if (current_jobs_.count(std::make_pair(parser_job.uuid, parser_job.is_error_job)))
    {
        BlackLibraryCommon::LogWarn("parser_manager", "Already working on job: {}", parser_job);
        return 0;
    }
    else
    {
        current_jobs_.emplace(std::make_pair(parser_job.uuid, parser_job.is_error_job), job_status_t::JOB_MANAGER_QUEUED);
    }

    job_queue_.push(parser_job);

    return 0;
}

std::vector<ParserJobStatusTracker> ParserManager::GetCurrentJobList()
{
    std::vector<ParserJobStatusTracker> job_list;
    auto map_keys = current_jobs_.get_map_keys();

    for (const auto &key : map_keys)
    {
        ParserJobStatusTracker tracker;
        auto job = current_jobs_.find(key);
        tracker.uuid = job->first.first;
        tracker.is_error_job = job->first.second;
        tracker.job_status = job->second;
        job_list.emplace_back(tracker);
    }

    return job_list;
}

bool ParserManager::GetDone()
{
    return done_;
}

bool ParserManager::StillWorkingOn(const std::string &uuid)
{
    return current_jobs_.count(std::make_pair(uuid, true)) || current_jobs_.count(std::make_pair(uuid, false));
}

int ParserManager::AddWorker(parser_t parser_type, size_t num_parsers)
{
    BlackLibraryCommon::LogInfo("parser_manager", "Adding worker {} with {} parsers", GetParserName(parser_type), num_parsers);

    worker_map_.emplace(parser_type, std::make_shared<ParserWorker>(parser_factory_, config_, parser_type, num_parsers));

    return 0;
}

int ParserManager::RegisterDatabaseStatusCallback(const database_status_callback &callback)
{
    database_status_callback_ = callback;

    return 0;
}

int ParserManager::RegisterProgressNumberCallback(const progress_number_callback &callback)
{
    progress_number_callback_ = callback;

    return 0;
}

int ParserManager::RegisterVersionReadNumCallback(const version_read_num_callback &callback)
{
    version_read_num_callback_ = callback;

    return 0;
}

} // namespace parsers
} // namespace core
} // namespace black_library
