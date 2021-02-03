/**
 * BlackLibraryUrlPuller.hh
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARYURLPULLER_HH__
#define __BLACK_LIBRARY_BLACKLIBRARYURLPULLER_HH__

#include <vector>
#include <string>

namespace black_library {

class BlackLibraryUrlPuller
{
public:
    virtual std::vector<std::string> PullUrls() const = 0;
};

} // namespace black_library

#endif
