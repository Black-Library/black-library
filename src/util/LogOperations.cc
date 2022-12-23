/**
 * LogOperations.cc
 */

#include <string.h>

#include <algorithm>

#include <LogOperations.h>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace black_library {

namespace core {

namespace common {

int CloseAllLoggers()
{
    spdlog::drop_all();

    return 0;
}

int CloseLogger(const std::string &logger_name)
{
    auto logger = spdlog::get(logger_name);
    logger->debug("Closing log: {}", logger_name);
    spdlog::drop(logger_name);

    return 0;
}

int InitRotatingLogger(const std::string &logger_name, const std::string &log_path, bool debug_mode)
{
    auto log_dir = log_path;
    try
    {
        auto logger_check = spdlog::get(logger_name);
        if (logger_check)
        {
            logger_check->debug("Init logger {} already active", logger_name);
            return 0;
        }

        if (log_dir.empty())
        {
            log_dir = DefaultLogPath;
            std::cout << "Empty log dir given, using default: " << log_dir << std::endl;
        }

        // okay to pop_back(), string isn't empty
        if (log_dir.back() == '/')
            log_dir.pop_back();

        const auto complete_log_path = log_dir + "/" + logger_name + ".txt";

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        if (debug_mode)
            console_sink->set_level(spdlog::level::debug);
        else
            console_sink->set_level(spdlog::level::info);

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(complete_log_path, MAX_LOG_SIZE, MAX_LOG_FILES);
        file_sink->set_level(spdlog::level::debug);

        spdlog::logger logger(logger_name, {console_sink, file_sink});
        logger.set_level(spdlog::level::debug);
        logger.flush_on(spdlog::level::warn);
        spdlog::register_logger(std::make_shared<spdlog::logger>(logger));

        logger.info("Start log with name {} at {} with debug_mode: {}", logger_name, complete_log_path, debug_mode);
    }
    catch(const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}

} // namespace common
} // namespace core
} // namespace black_library
