/**
 * blacklibrary.hh
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_HH__
#define __BLACK_LIBRARY_BLACKLIBRARY_HH__

#include <string>
#include <unordered_map>
#include <vector>

namespace black_library {

class BlackLibrary {
public:
    explicit BlackLibrary();

    int Run();
    int RunOnce();
    int Stop();

private:
    int PullUrls();
    int CompareUrls();
    int UpdateUrls();
    int UpdateStaging();
    int ParseUrls();
    int UpdateEntries();

    std::vector<std::string> urls_;
    std::unordered_map<std::string, std::string> parse_urls_;

};

} // namespace black_library

#endif
