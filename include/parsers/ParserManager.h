/**
 * ParserManager.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_MANAGER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_MANAGER_H__

#include <atomic>
#include <bitset>
#include <unordered_map>
#include <vector>

#include <ConfigOperations.h>

#include "BlockingQueue.h"
#include "BlockingUnorderedMap.h"
#include "ThreadPool.h"

#include "Parser.h"
#include "ParserFactory.h"
#include "ParserWorker.h"
#include "ParserDbAdapter.h"

namespace black_library {

namespace core {

namespace parsers {

class ParserManager
{
public:
    explicit ParserManager(const njson &config, const std::shared_ptr<ParserDbAdapter> &db_adapter);
    ParserManager &operator = (ParserManager &&) = default;

    int Run();
    int RunOnce();
    int Stop();

    bool IsReady();

    int AddJob(const std::string &uuid, const std::string &url, const std::string &last_url);
    int AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number);
    int AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number, const size_t &end_number);
    int AddJob(const std::string &uuid, const std::string &url, const std::string &last_url, const size_t &start_number, const size_t &end_number,  const error_job_rep &is_error_job);
    std::vector<ParserJobStatusTracker> GetCurrentJobList();
    bool GetDone();
    bool StillWorkingOn(const std::string &uuid);

    int RegisterDatabaseStatusCallback(const database_status_callback &callback);

    int RegisterProgressNumberCallback(const progress_number_callback &callback);
    int RegisterVersionReadNumCallback(const version_read_num_callback &callback);

private:
    int AddResult(ParserJobResult result);
    int AddWorker(parser_t parser_type, size_t num_parsers);

    std::unordered_map<parser_t, std::shared_ptr<ParserWorker>> worker_map_;
    std::shared_ptr<ParserFactory> parser_factory_;
    BlockingUnorderedMap<std::pair<std::string, error_job_rep>, job_status_t, CurrentJobPairHash> current_jobs_;
    BlockingQueue<ParserJob> job_queue_;
    BlockingQueue<ParserJobResult> result_queue_;

    std::shared_ptr<ParserDbAdapter> db_adapter_;

    database_status_callback database_status_callback_;
    progress_number_callback progress_number_callback_;
    version_read_num_callback version_read_num_callback_;

    njson config_;
    std::atomic_bool done_;
    std::atomic_bool initialized_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
