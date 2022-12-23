/**
 * FileOperations.cc
 */

#if __has_include(<filesystem>)
    #include <filesystem>
    namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
    #include <experimental/filesystem> 
    namespace fs = std::experimental::filesystem;
#else
    error "Missing the <filesystem> header."
#endif

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include <FileOperations.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace common {

// wrapper for access() that returns true if invoking user can write to path
bool CheckFilePermission(const std::string &target_path)
{
    if (access(target_path.c_str(), W_OK) == 0)
        return true;

    return false;
}

std::string GetMD5HashFromFile(const std::string &target_path)
{
    if (target_path.empty())
        return EmptyMD5Version;

    if (!PathExists(target_path))
        return EmptyMD5Version;

    std::string file_contents;

    std::ifstream input_file(target_path, std::ios::in | std::ios::binary);

    if (!input_file)
        return EmptyMD5Version;

    input_file.seekg(0, std::ios::end);
    file_contents.resize(input_file.tellg());
    input_file.seekg(0, std::ios::beg);
    input_file.read(&file_contents[0], file_contents.size());
    input_file.close();

    return GetMD5Hash(file_contents);
}

std::vector<std::string> GetFileList(const std::string &target_path)
{
    return GetFileList(target_path, "*");
}

// wrapper for std::filesystem list directorty contents
std::vector<std::string> GetFileList(const std::string &target_path, const std::string &regex_string)
{
    std::vector<std::string> file_list;

    if (!PathExists(target_path))
        return file_list;

    try
    {
        const std::regex regex(regex_string, std::regex::extended); // POSIX
        for (const auto & entry : fs::directory_iterator(target_path))
        {
            const auto file_name = entry.path().filename().string();

            if (std::regex_search(file_name, regex))
            {
                file_list.emplace_back(file_name);
            }
        }
    }
    catch (std::regex_error &e)
    {
        std::cout << "Error: regex failed" << std::endl;
    }

    return file_list;
}

// wrapper for std::filesystem::create_directories
bool MakeDirectories(const std::string &target_path)
{
    if (fs::exists(target_path))
    {
        return true;
    }

    return fs::create_directories(target_path);
}

// wrapper for std::filesystem::exists
bool PathExists(const std::string &target_path)
{
    return fs::exists(target_path);
}

bool PathExistsAndPermission(const std::string &target_path)
{
    return PathExists(target_path) && CheckFilePermission(target_path);
}

// wrapper for std::filesystem::remove
bool RemovePath(const std::string &target_path)
{
    return fs::remove_all(target_path);
}

std::string SanitizeFileName(const std::string &file_name)
{
    std::string sanatized_file_name = file_name;
    const std::string unallowed = " /\\*?<>:;=[]!@|.,%#'\"";

    while (sanatized_file_name.front() == '-')
    {
        sanatized_file_name.erase(sanatized_file_name.begin());
    }

    while (sanatized_file_name.back() == '-' && sanatized_file_name.size() > 0)
    {
        sanatized_file_name.pop_back();
    }

    std::transform(sanatized_file_name.begin(), sanatized_file_name.end(), sanatized_file_name.begin(),
    [&unallowed](char ch)
    {
        return (std::find(unallowed.begin(), unallowed.end(), ch) != unallowed.end()) ? '-' : ch;
    });

    const std::string dup_string = "--";

    size_t pos = sanatized_file_name.find(dup_string);

    while (pos != std::string::npos)
    {
        sanatized_file_name.replace(pos, dup_string.size(), "-");
        pos = sanatized_file_name.find(dup_string);
    }

    if (sanatized_file_name.back() == '-')
        sanatized_file_name.pop_back();

    return sanatized_file_name;
}

std::string SanitizeFilePath(const std::string &file_name)
{
    std::string sanatized_file_name = file_name;
    const std::string unallowed = " *?<>:;=[]!@|";

    if (sanatized_file_name.front() == '-')
    {
        sanatized_file_name.erase(sanatized_file_name.begin());
    }

    std::transform(sanatized_file_name.begin(), sanatized_file_name.end(), sanatized_file_name.begin(),
    [&unallowed](char ch)
    {
        return (std::find(unallowed.begin(), unallowed.end(), ch) != unallowed.end()) ? '-' : ch;
    });

    return sanatized_file_name;
}

} // namespace common
} // namespace core
} // namespace black_library
