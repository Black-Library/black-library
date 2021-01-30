/**
 * BlackLibrary.hh
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_HH__
#define __BLACK_LIBRARY_BLACKLIBRARY_HH__

#include <string>
#include <unordered_map>
#include <vector>

#include <BlackLibraryDB.hh>

namespace black_library {

class BlackLibrary {
public:
    explicit BlackLibrary(const std::string &db_url);

    BlackLibrary &operator = (BlackLibrary &&) = default;

    int Run();
    int RunOnce();
    int Stop();

private:

    int Init();
    int PullUrls();
    int CompareUrls();
    int UpdateUrls();
    int UpdateStaging();
    int ParseUrls();
    int UpdateEntries();
    int CleanStaging();

    black_library::core::db::BlackLibraryDB blacklibrarydb_;
    std::unordered_map<std::string, std::string> parse_urls_;
    std::vector<std::string> urls_;
    bool done_;
};

} // namespace black_library

#endif
