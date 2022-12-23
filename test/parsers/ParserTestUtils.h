/**
 * ParserTestUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_TEST_UTILS_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_TEST_UTILS_H__

#include <ConfigOperations.h>

namespace black_library {

namespace core {

namespace parsers {

static constexpr const char DefaultTestLogPath[] = "/tmp/log";
static constexpr const char DefaultTestStoragePath[] = "/tmp/store";

static constexpr const char RR_DUMMY_URL[] = "https://www.royalroad.com/fiction/00000/some-fiction";

njson GenerateParserTestConfig()
{
    njson j;

    j["config"]["logger_path"] = DefaultTestLogPath;
    j["config"]["storage_path"] = DefaultTestStoragePath;
    j["config"]["worker_debug_log"] = true;
    j["config"]["parser_debug_log"] = true;

    return j;
}

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
