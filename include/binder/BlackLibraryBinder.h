/**
 * BlackLibraryBinder.h
 */

#ifndef __BLACK_LIBRARY_CORE_BINDER_BLACKLIBRARYBINDER_H__
#define __BLACK_LIBRARY_CORE_BINDER_BLACKLIBRARYBINDER_H__

#include <mutex>
#include <string>

#include <ConfigOperations.h>

namespace black_library {

namespace core {

namespace binder {

static constexpr const char DefaultBindPath[] = "/mnt/black-library/output/";

class BlackLibraryBinder {
public:
    explicit BlackLibraryBinder(const njson &config);
    BlackLibraryBinder &operator = (BlackLibraryBinder &&) = default;

    bool Bind(const std::string &uuid, const std::string &name);
    bool SetBindDir(const std::string &bind_dir);

private:
    std::string bind_path_;
    std::string storage_path_;
    std::mutex mutex_;
};

} // namespace binder
} // namespace core
} // namespace black_library

#endif
