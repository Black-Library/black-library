/**
 * StringOperations.cc
 */

#include <string.h>

#include <algorithm>

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
