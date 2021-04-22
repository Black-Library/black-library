/**
 * BlackLibrary.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_H__

#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <BlackLibraryUrlPuller.h>

#include <BlackLibraryDB.h>
#include <BlackLibraryDBConnectionInterface.h>

#include <ParserManager.h>

namespace black_library {

class BlackLibrary {
public:
    explicit BlackLibrary(const std::string &db_url, bool init_db);

    BlackLibrary &operator = (BlackLibrary &&) = default;

    int Run();
    int RunOnce();
    int Stop();

private:

    int Init();
    int PullUrls();
    int VerifyUrls();
    int CompareAndUpdateUrls();
    int UpdateStaging();
    int ParseUrls();

    int UpdateDatabaseWithResult(core::db::DBEntry &entry, const core::parsers::ParserJobResult &result);

    std::string GenerateUUID();
    int PrintTabbed(const std::string &statment, size_t num_tabs);

    black_library::core::parsers::ParserManager parser_manager_;
    black_library::core::db::BlackLibraryDB blacklibrary_db_;
    std::shared_ptr<black_library::BlackLibraryUrlPuller> url_puller_;
    std::vector<black_library::core::db::DBEntry> parse_entries_;
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
