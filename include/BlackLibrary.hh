/**
 * BlackLibrary.hh
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_HH__
#define __BLACK_LIBRARY_BLACKLIBRARY_HH__

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <BlackLibraryUrlPuller.hh>

#include <BlackLibraryDB.hh>
#include <BlackLibraryDBConnectionInterface.hh>

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
    int CompareAndUpdateUrls();
    int UpdateStaging();
    int ParseUrls();
    int UpdateEntries();
    int CleanStaging();

    std::string GenerateUUID();

    black_library::core::db::BlackLibraryDB blacklibrarydb_;
    std::shared_ptr<black_library::BlackLibraryUrlPuller> url_puller_;
    std::unordered_map<std::string, std::string> parse_urls_;
    std::vector<std::string> urls_;
    std::random_device rd_;
    std::mt19937_64 gen_;
    std::uniform_int_distribution<> dist0_;
    std::uniform_int_distribution<> dist1_;

    bool done_;
};

} // namespace black_library

#endif
