/**
 * BlackLibraryBinder.cc
 */

#include <algorithm>
#include <iostream>
#include <fstream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <StringOperations.h>

#include <BlackLibraryBinder.h>

namespace black_library {

namespace core {

namespace binder {

#define BIND_FILENAME_BUFFER_SIZE 128

namespace BlackLibraryCommon = black_library::core::common;

size_t GetBindIndex(const std::string &file_name)
{
    if (file_name.empty())
        return 0;

    auto first_pos = file_name.find_last_of("_");
    auto second_pos = file_name.find_last_of('.');

    if (first_pos == std::string::npos || second_pos == std::string::npos)
        return 0;

    auto temp_string = file_name.substr(first_pos, second_pos - first_pos);

    first_pos = temp_string.find_last_not_of("0123456789");

    temp_string = temp_string.substr(first_pos + 1, temp_string.size());

    return std::stoull(temp_string);
}

size_t GetSectionIndex(const std::string &file_name)
{
    if (file_name.empty())
        return 0;

    auto first_pos = file_name.find_first_of("0123456789");
    auto second_pos = file_name.find_first_of('_');

    if (first_pos == std::string::npos || second_pos == std::string::npos)
        return 0;

    auto temp_string = file_name.substr(first_pos, second_pos);

    return std::stoull(temp_string);
}

BlackLibraryBinder::BlackLibraryBinder(const njson &config) :
    bind_path_(DefaultBindPath),
    storage_path_(BlackLibraryCommon::DefaultStoragePath),
    logger_name_("binder"),
    mutex_()
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    bool logger_level = BlackLibraryCommon::DefaultLogLevel;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    if (nconfig.contains("binder_debug_log"))
    {
        logger_level = nconfig["binder_debug_log"];
    }

    if (nconfig.contains("binder_dir"))
    {
        bind_path_ = nconfig["binder_dir"];
    }

    if (nconfig.contains("storage_path"))
    {
        storage_path_ = nconfig["storage_path"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, logger_level);

    // okay to pop_back(), string isn't empty
    if (storage_path_.back() == '/')
        storage_path_.pop_back();

    if (storage_path_.empty())
    {
        storage_path_ = BlackLibraryCommon::DefaultStoragePath;
        BlackLibraryCommon::LogDebug(logger_name_, "Empty storage dir given, using default: {}", storage_path_);
    }

    if (!BlackLibraryCommon::CheckFilePermission(storage_path_))
    {
        BlackLibraryCommon::LogError(logger_name_, "Could not access storage directory: {}", storage_path_);
        return;
    }

    BlackLibraryCommon::LogInfo(logger_name_, "Binder using storage dir: {}", storage_path_);
}

bool BlackLibraryBinder::Bind(const std::string &uuid, const std::string &name)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    // check bind source and target
    std::string target_path = storage_path_ + "/" + uuid;
    BlackLibraryCommon::LogInfo(logger_name_, "Bind target: {}", target_path);

    if (!BlackLibraryCommon::PathExists(target_path))
    {
        BlackLibraryCommon::LogError(logger_name_, "Binder target does not exist: {}", target_path);
        return false;
    }

    // unsorted list, we should not trust the names will be exactly similar
    auto sec_files = BlackLibraryCommon::GetFileList(target_path, "^SEC?\\d*");

    // map to store data, avoid repeated regex
    std::unordered_map<std::string, int16_t> section_map;
    std::unordered_map<std::string, int16_t> version_map;

    for (const auto& file : sec_files)
    {
        section_map.emplace(file, BlackLibraryCommon::GetSectionNumFromFileName(file));
        version_map.emplace(file, BlackLibraryCommon::GetVersionNumFromFileName(file));
    }

    // sort files by section and then version   
    std::sort(sec_files.begin(), sec_files.end(),
        [section_map, version_map](const auto &a, const auto &b)
        {
            auto a_sec_iter = section_map.find(a);
            auto b_sec_iter = section_map.find(b);
            if (a_sec_iter->second != b_sec_iter->second)
                return a_sec_iter->second < b_sec_iter->second;
            
            auto a_ver_iter = version_map.find(a);
            auto b_ver_iter = version_map.find(b);
            return a_ver_iter->second < b_ver_iter->second;
        }
    );

    BlackLibraryCommon::LogDebug(logger_name_, "total sorted files", sec_files.size());
    BlackLibraryCommon::LogDebug(logger_name_, "front and end from sorted files: {} - {}", sec_files.front(), sec_files.back());

    // get vector of latest files
    std::vector<std::string> latest_version_sec_files;
    for (size_t i = 0; i < sec_files.size(); ++i)
    {
        // BlackLibraryCommon::LogDebug(logger_name_, "reached count {}", i);
        if (i + 1 < sec_files.size())
        {
            auto curr_sec_iter = section_map.find(sec_files[i]);
            auto next_sec_iter = section_map.find(sec_files[i + 1]);
            if (curr_sec_iter != section_map.end() && next_sec_iter != section_map.end())
            {
                if (curr_sec_iter->second == next_sec_iter->second)
                {
                    continue;
                }
            }
            else
            {
                BlackLibraryCommon::LogError(logger_name_, "could not find {} or {}", sec_files[i], sec_files[i + 1]);
            }
        }
        latest_version_sec_files.emplace_back(sec_files[i]);
    }

    BlackLibraryCommon::LogDebug(logger_name_, "front and end from latest files: {} - {}", latest_version_sec_files.front(), latest_version_sec_files.back());

    for (const auto & file : latest_version_sec_files)
    {
        BlackLibraryCommon::LogTrace(logger_name_, "Section file: {} - Number: {}", file, GetSectionIndex(file));
    }

    size_t last_section_num = GetSectionIndex(latest_version_sec_files.back());

    // document files start at 1
    if (last_section_num != latest_version_sec_files.size())
    {
        BlackLibraryCommon::LogError(logger_name_, "Section number {} - list size: {} do not match", last_section_num, latest_version_sec_files.size());
        return false;
    }

    size_t bind_index = 0;

    if (!latest_version_sec_files.empty())
        bind_index = GetBindIndex(latest_version_sec_files.back()) + 1;

    char name_buffer [BIND_FILENAME_BUFFER_SIZE];

    int res = snprintf(name_buffer, BIND_FILENAME_BUFFER_SIZE, "%s_VER%04lu.html", name.c_str(), bind_index);
    if (res < 0)
        return false;

    const std::string bind_name = std::string(name_buffer);
    const std::string bind_target = bind_path_ + '/' + bind_name;

    BlackLibraryCommon::LogInfo(logger_name_, "Binding: {}", bind_name);

    if (BlackLibraryCommon::PathExists(bind_target))
    {
        BlackLibraryCommon::LogError(logger_name_, "File {} already exists", bind_target);
        return false;
    }

    std::fstream output_file;

    output_file.open(bind_target, std::fstream::out | std::fstream::trunc);

    output_file << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><style></style></head><body>";

    for (const auto & file : latest_version_sec_files)
    {
        std::ifstream input_file;

        BlackLibraryCommon::LogTrace(logger_name_, "Bind: {}", file);

        const auto complete_file_path = target_path + '/' + file;

        input_file.open(complete_file_path, std::fstream::in);

        if (!input_file.is_open())
        {
            BlackLibraryCommon::LogError(logger_name_, "Input file: {} failed to open", complete_file_path);
            return false;
        }

        output_file << input_file.rdbuf();

        input_file.close();
    }

    output_file << "</body></html>";

    output_file.close();

    return true;
}

bool BlackLibraryBinder::SetBindDir(const std::string &bind_dir)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    bind_path_ = BlackLibraryCommon::SanitizeFilePath(bind_dir);

    if (bind_path_.empty())
    {
        bind_path_ = DefaultBindPath;
        BlackLibraryCommon::LogDebug(logger_name_, "Empty bind_dir given, using default: {}", bind_path_);
    }

    // okay to pop_back(), string isn't empty
    if (bind_path_.back() == '/')
        bind_path_.pop_back();

    if (!BlackLibraryCommon::CheckFilePermission(bind_path_))
    {
        BlackLibraryCommon::LogError(logger_name_, "Binder could not access bind directory: {}", bind_path_);
        return false;
    }

    BlackLibraryCommon::LogInfo(logger_name_, "Set bind_dir: {}", bind_path_);

    return true;
}

} // namespace binder
} // namespace core
} // namespace black_library