/**
 * StringOperations.cc
 */

#include <string.h>

#include <algorithm>
#include <charconv>
#include <iostream>
#include <regex>

#include <StringOperations.h>

namespace black_library {

namespace core {

namespace common {

// C++ wrapper around strstr()
bool ContainsString(const std::string &haystack, const std::string &needle)
{
    // C-style compare is fastest
    if (strstr(haystack.c_str(), needle.c_str()) == NULL)
    {
        return false;
    }

    return true;
}

int16_t GetSectionNumFromFileName(const std::string &target_path)
{
    try
    {
        const std::regex regex("^SEC(\\d{4})_");
        std::smatch matches;
        if (std::regex_search(target_path, matches, regex))
        {
            std::string candidate = matches[1].str();
            int16_t sec_num = 0;

            auto [end, eror_code] = std::from_chars(candidate.data(), candidate.data() + candidate.size(), sec_num);

            if (eror_code == std::errc::result_out_of_range) {
                std::string error_str = "integer is too large: " + std::to_string(candidate.size()) + " - " + candidate;
                throw std::out_of_range(error_str);
            }
            if (eror_code != std::errc{} || end != candidate.data() + candidate.size()) {
                std::string error_str = "invalid integer: " + candidate;
                throw std::invalid_argument(error_str);
            }

            return sec_num;
        }
    }
    catch (const std::regex_error &ex)
    {
        std::cerr << "Invalid regex: " << ex.what() << '\n';
    }
    catch (const std::out_of_range &ex)
    {
        std::cerr << "Out of range error: " << ex.what() << '\n';
    }
    catch (const std::invalid_argument &ex)
    {
        std::cerr << "Error with integer" << ex.what() << '\n';
    }

    return -1;
}

int16_t GetVersionNumFromFileName(const std::string &target_path)
{
    try
    {
        const std::regex regex("_VER(\\d{3})\\.");
        std::smatch matches;
        if (std::regex_search(target_path, matches, regex))
        {
            std::string candidate = matches[1].str();
            int16_t sec_num = 0;

            auto [end, eror_code] = std::from_chars(candidate.data(), candidate.data() + candidate.size(), sec_num);

            if (eror_code == std::errc::result_out_of_range) {
                std::string error_str = "integer is too large: " + std::to_string(candidate.size()) + " - " + candidate;
                throw std::out_of_range(error_str);
            }
            if (eror_code != std::errc{} || end != candidate.data() + candidate.size()) {
                std::string error_str = "invalid integer: " + candidate;
                throw std::invalid_argument(error_str);
            }

            return sec_num;
        }
    }
    catch (const std::regex_error &ex)
    {
        std::cerr << "Invalid regex: " << ex.what() << '\n';
    }
    catch (const std::out_of_range &ex)
    {
        std::cerr << "Out of range error: " << ex.what() << '\n';
    }
    catch (const std::invalid_argument &ex)
    {
        std::cerr << "Error with integer" << ex.what() << '\n';
    }

    return -1;
}

void SanatizeString(std::string &target_string)
{
    target_string.erase(std::remove(target_string.begin(), target_string.end(), '\r'), target_string.end());
    target_string.erase(std::remove(target_string.begin(), target_string.end(), '\n'), target_string.end());
    target_string.erase(std::remove(target_string.begin(), target_string.end(), ' '), target_string.end());
    target_string.erase(std::remove(target_string.begin(), target_string.end(), '\n'), target_string.end());
    target_string.erase(std::remove(target_string.begin(), target_string.end(), '\0'), target_string.end());
    target_string.erase(std::remove(target_string.begin(), target_string.end(), ';'), target_string.end());

    // remove unallowed strings
    const std::string remove_str = "’";
    const auto pos = target_string.find(remove_str);

    if (pos != std::string::npos)
        target_string.erase(pos, remove_str.length());

    // remove unprintable characters
    target_string.erase(std::remove_if(target_string.begin(), target_string.end(),
        [](unsigned char c)
        {
            return !std::isprint(c);
        })
        , target_string.end());
}

// C++ wrapper around strncmp
bool StartsWithString(const std::string &haystack, const std::string &needle)
{
    if (strncmp(haystack.c_str(), needle.c_str(), strlen(needle.c_str())))
    {
        return false;
    }

    return true;
}

std::string SubstringAfterString(const std::string &base_string, const std::string &substring_string)
{
    const auto pos = base_string.find(substring_string);
    if (pos == base_string.npos)
        return base_string;
    return base_string.substr(pos + substring_string.size());
}

std::string TrimWhitespace(const std::string &target_string)
{
    auto leading_pos = target_string.find_first_not_of(" \t\r\n\0");
    auto trailing_pos = target_string.find_last_not_of(" \t\r\n\0");
    if (leading_pos == std::string::npos && trailing_pos == std::string::npos)
    {
        return "";
    }

    return target_string.substr(leading_pos, trailing_pos - leading_pos + 1);
}

} // namespace common
} // namespace core
} // namespace black_library
