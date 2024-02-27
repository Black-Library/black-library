/**
 * BlackLibrary.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_H__

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <ConfigOperations.h>
#include <BlackLibraryDB.h>
#include <BlackLibraryRESTAPI.h>
#include <ParserManager.h>

#include <BlackLibraryUrlPuller.h>
#include <SimpleUUIDGenerator.h>

namespace black_library {

namespace BlackLibraryDB = black_library::core::db;
namespace BlackLibraryParsers = black_library::core::parsers;
namespace BlackLibraryRESTAPI = black_library::core::rest_api;

class BlackLibrary {
public:
    explicit BlackLibrary(const njson &json);

    BlackLibrary &operator = (BlackLibrary &&) = default;

    int Run();
    int RunOnce();
    int Stop();

private:
    int PullUrls();
    int VerifyUrls();
    int CompareAndUpdateUrls();
    int ParseUrls();
    int ParseErrorEntries();

    int UpdateDatabaseWithResult(BlackLibraryDB::DBEntry &entry, const BlackLibraryParsers::ParserJobResult &result);

    std::string GenerateUUID();

    std::shared_ptr<BlackLibraryParsers::ParserManager> parser_manager_;
    std::shared_ptr<BlackLibraryDB::BlackLibraryDB> blacklibrary_db_;
    std::shared_ptr<BlackLibraryRESTAPI::BlackLibraryDBRESTAPI> blacklibrary_api_;
    std::shared_ptr<black_library::BlackLibraryUrlPuller> url_puller_;
    std::shared_ptr<black_library::SimpleUUIDGenerator> uuid_gen_;
    std::vector<BlackLibraryDB::DBEntry> parse_entries_;
    std::vector<std::string> pull_urls_;
    std::thread manager_thread_;
    std::string logger_name_;
    std::mutex database_parser_mutex_;
    bool debug_target_;
    bool enable_api_;

    std::atomic_bool done_;
};

} // namespace black_library

#endif
