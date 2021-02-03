/**
 * WgetUrlPuller.hh
 */

#ifndef __BLACK_LIBRARY_WGETURLPULLER_HH__
#define __BLACK_LIBRARY_WGETURLPULLER_HH__

#include <BlackLibraryUrlPuller.hh>

namespace black_library {

class WgetUrlPuller : public BlackLibraryUrlPuller
{
public:
    explicit WgetUrlPuller();

    std::vector<std::string> PullUrls() const override;
};

} // namespace black_library

#endif
