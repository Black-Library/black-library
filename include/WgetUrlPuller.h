/**
 * WgetUrlPuller.h
 */

#ifndef __BLACK_LIBRARY_WGETURLPULLER_H__
#define __BLACK_LIBRARY_WGETURLPULLER_H__

#include <BlackLibraryUrlPuller.h>

namespace black_library {

class WgetUrlPuller : public BlackLibraryUrlPuller
{
public:
    std::vector<std::string> PullUrls() const override;
};

} // namespace black_library

#endif
