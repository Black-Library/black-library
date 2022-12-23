/**
 * LogOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_LOG_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_LOG_OPERATIONS_H__

#include <iostream>
#include <string>

#include "spdlog/spdlog.h"

namespace black_library {

namespace core {

namespace common {

#define MAX_LOG_SIZE 1048576
#define MAX_LOG_FILES 3

static constexpr const char DefaultLogPath[] = "/mnt/black-library/log";
static const bool DefaultLogLevel = false;

int CloseAllLoggers();
int CloseLogger(const std::string &logger_name);
int InitRotatingLogger(const std::string &logger_name, const std::string &log_path, bool debug_mode);

template<typename ... Args>
inline void LogTrace(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->trace(format, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void LogDebug(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->debug(format, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void LogInfo(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->info(format, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void LogWarn(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->warn(format, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void LogError(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->error(format, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void LogCritical(const std::string &logger_name, const std::string &format, Args &&...args)
{
    auto logger = spdlog::get(logger_name);
    
    if (!logger)
        std::cerr << "Error: logger " << logger_name << " does not exist" << std::endl;
    else
        logger->critical(format, std::forward<Args>(args)...);
}

} // namespace common
} // namespace core
} // namespace black_library

#endif
