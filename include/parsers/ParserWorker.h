/**
 * ParserWorker.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_WORKER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_WORKER_H__

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include <ConfigOperations.h>

#include "BlockingQueue.h"
#include "ThreadPool.h"

#include "Parser.h"
#include "ParserFactory.h"

namespace black_library {

namespace core {

namespace parsers {

class ParserWorker
{
public:
    explicit ParserWorker(const std::shared_ptr<ParserFactory> parser_factory_, const njson &config, parser_t parser_type, size_t num_parsers);
    ParserWorker &operator = (ParserWorker &&) = default;

    int Run();
    int RunOnce();
    int Stop();

    int AddJob(const ParserJob &parser_job);

    int RegisterProgressNumberCallback(const progress_number_callback &callback);
    int RegisterJobStatusCallback(const job_status_callback &callback);
    int RegisterManagerNotifyCallback(const manager_notify_callback &callback);

    int RegisterMd5CheckCallback(const md5_check_callback &callback);
    int RegisterMd5sReadCallback(const md5s_read_callback &callback);
    int RegisterMd5ReadCallback(const md5_read_callback &callback);
    int RegisterVersionReadNumCallback(const version_read_num_callback &callback);
    int RegisterMd5UpdateCallback(const md5_update_callback &callback);

private:
    void Init();

    ThreadPool pool_;
    BlockingQueue<ParserJob> job_queue_;
    std::vector<std::future<ParserJobResult>> pool_results_;
    std::priority_queue<size_t> pool_erases_;


    job_status_callback job_status_callback_;
    manager_notify_callback notify_callback_;

    md5_check_callback md5_check_callback_;
    md5_read_callback md5_read_callback_;
    md5s_read_callback md5s_read_callback_;
    md5_update_callback md5_update_callback_;

    progress_number_callback progress_number_callback_;
    version_read_num_callback version_read_num_callback_;

    std::string storage_path_;
    std::string worker_name_;
    std::shared_ptr<ParserFactory> parser_factory_;
    std::mutex mutex_;
    parser_t parser_type_;
    std::atomic_bool done_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
