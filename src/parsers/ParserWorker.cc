/**
 * ParserWorker.cc
 */

#include <iostream>
#include <sstream>

#include <FileOperations.h>
#include <LogOperations.h>

#include <ParserWorker.h>

#include <ParserRR.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

ParserWorker::ParserWorker(const std::shared_ptr<ParserFactory> parser_factory, const njson &config, parser_t parser_type, size_t num_parsers) :
    pool_(num_parsers),
    job_queue_(),
    pool_results_(),
    pool_erases_(),
    job_status_callback_(),
    notify_callback_(),
    md5_check_callback_(),
    md5_read_callback_(),
    md5s_read_callback_(),
    md5_update_callback_(),
    progress_number_callback_(),
    version_read_num_callback_(),
    storage_path_(BlackLibraryCommon::DefaultStoragePath),
    worker_name_(""),
    parser_factory_(parser_factory),
    parser_type_(parser_type),
    done_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    bool logger_level = BlackLibraryCommon::DefaultLogLevel;
    if (nconfig.contains("worker_debug_log"))
    {
        logger_level = nconfig["worker_debug_log"];
    }

    if (nconfig.contains("storage_path"))
    {
        storage_path_ = nconfig["storage_path"];
    }

    // okay to pop_back(), string isn't empty
    if (storage_path_.back() == '/')
        storage_path_.pop_back();

    worker_name_ = GetParserName(parser_type_) + "_worker";

    BlackLibraryCommon::InitRotatingLogger(worker_name_, logger_path, logger_level);

    BlackLibraryCommon::LogInfo(worker_name_, "Initialize parser worker: {} with pool size: {}", GetParserName(parser_type), pool_.GetSize());
}

int ParserWorker::Run()
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

int ParserWorker::RunOnce()
{
    const std::lock_guard<std::mutex> lock(mutex_);

    // clear pool_erases
    while (!pool_erases_.empty())
    {
        pool_erases_.pop();
    }

    // check if futures list is ready
    for (size_t i = 0; i < pool_results_.size(); ++i)
    {
        auto & result = pool_results_[i];

        // check if future is ready
        if (!result.valid())
            continue;

        if (result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            ParserJobResult job_result = result.get();

            if (job_result.has_error)
            {
                BlackLibraryCommon::LogError(worker_name_, "Job result returned error: {}", job_result);
                pool_erases_.push(i);
            }
            else
            {
                if (notify_callback_)
                    notify_callback_(job_result);
                pool_erases_.push(i);
            }
        }
    }

    while (!pool_erases_.empty())
    {
        size_t pos = pool_erases_.top();
        pool_erases_.pop();
        pool_results_.erase(pool_results_.begin() + pos);
    }

    if (job_queue_.empty())
        return 0;

    auto parser_job = job_queue_.pop();

    pool_results_.emplace_back(
        pool_.enqueue([this, parser_job]()
        {
            std::stringstream ss;
            ParserJobResult job_result;
            std::atomic_bool parser_error;

            if (done_)
                return job_result;

            parser_error = false;

            job_result.metadata.url = parser_job.url;
            job_result.metadata.uuid = parser_job.uuid;

            auto factory_result = parser_factory_->GetParserByUrl(parser_job.url);

            if (factory_result.has_error)
            {
                BlackLibraryCommon::LogError(worker_name_, "Factory failed to match url");
                return job_result;
            }

            auto parser = factory_result.parser_result;

            std::string local_file_path = storage_path_ + '/' + parser_job.uuid + '/';

            parser->SetLocalFilePath(local_file_path);

            if (!BlackLibraryCommon::CheckFilePermission(storage_path_))
            {
                BlackLibraryCommon::LogError(worker_name_, "Failed to access storage directory: {}", storage_path_);
                return job_result;
            }

            if (!BlackLibraryCommon::MakeDirectories(local_file_path))
            {
                BlackLibraryCommon::LogError(worker_name_, "Failed to make local file path directory: {}", local_file_path);
                return job_result;
            }

            if (!BlackLibraryCommon::CheckFilePermission(local_file_path))
            {
                BlackLibraryCommon::LogError(worker_name_, "Failed to access UUID directory: {}", local_file_path);
                return job_result;
            }

            BlackLibraryCommon::LogDebug(worker_name_, "Starting parser: {} job: {}", GetParserName(parser->GetParserType()), parser_job);

            std::thread t([this, parser, &parser_job, &parser_error](){

                while (!done_ && !parser->GetDone() && !parser_error)
                {
                    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);

                    if (done_)
                        break;

                    std::this_thread::sleep_until(deadline);
                }

                BlackLibraryCommon::LogDebug(worker_name_, "Job: {} done", parser_job);

                parser->Stop();
            });

            if (job_status_callback_)
                job_status_callback_(parser_job, job_status_t::JOB_WORKING);

            if (md5_check_callback_)
                parser->RegisterMd5CheckCallback(md5_check_callback_);

            if (md5_read_callback_)
                parser->RegisterMd5ReadCallback(md5_read_callback_);

            if (md5s_read_callback_)
                parser->RegisterMd5sReadCallback(md5s_read_callback_);

            if (md5_update_callback_)
                parser->RegisterMd5UpdateCallback(md5_update_callback_);

            if (progress_number_callback_)
                parser->RegisterProgressNumberCallback(progress_number_callback_);

            if (version_read_num_callback_)
                parser->RegisterVersionReadNumCallback(version_read_num_callback_);

            auto parser_result = parser->Parse(parser_job);

            if (parser_result.has_error)
            {
                parser_error = true;
                if (job_status_callback_)
                    job_status_callback_(parser_job, job_status_t::JOB_ERROR);
            }
            else
            {
                if (job_status_callback_)
                    job_status_callback_(parser_job, job_status_t::JOB_FINISHED);
            }

            t.join();

            BlackLibraryCommon::LogDebug(worker_name_, "Stopping parser: {} job: {}", GetParserName(parser->GetParserType()), parser_job);

            job_result.metadata = parser_result.metadata;
            job_result.start_number = parser_job.start_number;
            job_result.end_number = parser_job.end_number;
            job_result.is_error_job = parser_job.is_error_job;

            if (!parser_result.has_error)
                job_result.has_error = false;

            return job_result;
        })
    );

    return 0;
}

int ParserWorker::Stop()
{
    const std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;

    for (auto & result : pool_results_)
    {
        if (result.valid())
        {
            BlackLibraryCommon::LogDebug(worker_name_, "Stopping a job");
            result.wait();
        }
    }

    BlackLibraryCommon::LogInfo(worker_name_, "Stopped worker with: {} remaining jobs", pool_results_.size());

    BlackLibraryCommon::CloseLogger(worker_name_);

    return 0;
}

int ParserWorker::AddJob(const ParserJob &parser_job)
{
    if (parser_job.uuid.empty())
    {
        BlackLibraryCommon::LogError(worker_name_, "Sent job with empty uuid, job: {}", parser_job);
        return -1;
    }

    if (parser_job.url.empty())
    {
        BlackLibraryCommon::LogError(worker_name_, "Sent job with empty url, job: {}", parser_job);
        return -1;
    }

    BlackLibraryCommon::LogDebug(worker_name_, "Adding job: {}", parser_job);

    if (job_status_callback_)
        job_status_callback_(parser_job, job_status_t::JOB_WORKER_QUEUED);

    job_queue_.push(parser_job);

    return 0;
}

int ParserWorker::RegisterJobStatusCallback(const job_status_callback &callback)
{
    job_status_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterManagerNotifyCallback(const manager_notify_callback &callback)
{
    notify_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterMd5CheckCallback(const md5_check_callback &callback)
{
    md5_check_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterMd5ReadCallback(const md5_read_callback &callback)
{
    md5_read_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterMd5sReadCallback(const md5s_read_callback &callback)
{
    md5s_read_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterMd5UpdateCallback(const md5_update_callback &callback)
{
    md5_update_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterProgressNumberCallback(const progress_number_callback &callback)
{
    progress_number_callback_ = callback;

    return 0;
}

int ParserWorker::RegisterVersionReadNumCallback(const version_read_num_callback &callback)
{
    version_read_num_callback_ = callback;

    return 0;
}

} // namespace parsers
} // namespace core
} // namespace black_library
