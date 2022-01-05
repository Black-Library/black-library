/**
 * BlackLibrary.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_H__

#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <ConfigOperations.h>
#include <BlackLibraryDB.h>
#include <ParserManager.h>

#include <BlackLibraryUrlPuller.h>

namespace black_library {

namespace BlackLibraryDB = black_library::core::db;
namespace BlackLibraryParsers = black_library::core::parsers;

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
    int UpdateStaging();
    int ParseUrls();
    int ParserErrorEntries();

    int UpdateDatabaseWithResult(BlackLibraryDB::DBEntry &entry, const BlackLibraryParsers::ParserJobResult &result);

    std::string GenerateUUID();

    BlackLibraryParsers::ParserManager parser_manager_;
    BlackLibraryDB::BlackLibraryDB blacklibrary_db_;
    std::shared_ptr<black_library::BlackLibraryUrlPuller> url_puller_;
    std::vector<BlackLibraryDB::DBEntry> parse_entries_;
    std::vector<std::string> pull_urls_;
    std::thread manager_thread_;
    std::random_device rd_;
    std::mt19937_64 gen_;
    std::uniform_int_distribution<> dist0_;
    std::uniform_int_distribution<> dist1_;
    std::mutex database_parser_mutex_;

    bool done_;
};

} // namespace black_library

#endif
