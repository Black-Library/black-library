/**
 * FileOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_STRING_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_STRING_OPERATIONS_H__

#include <stdint.h>
#include <string>

namespace black_library {

namespace core {

namespace common {

bool ContainsString(const std::string &haystack, const std::string &needle);
int16_t GetSectionNumFromFileName(const std::string &target_path);
int16_t GetVersionNumFromFileName(const std::string &target_path);
void SanatizeString(std::string &target_string);
bool StartsWithString(const std::string &haystack, const std::string &needle);
std::string SubstringAfterString(const std::string &base_string, const std::string &substring_string);
std::string TrimWhitespace(const std::string &target_string);

} // namespace common
} // namespace core
} // namespace black_library

#endif
