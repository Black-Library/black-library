/**
 * BlackLibraryCLI.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__

#include <atomic>

#include <BlackLibraryDB.h>

namespace black_library {

class BlackLibraryCLI {
public:
    explicit BlackLibraryCLI(const std::string &db_path, const std::string &storage_path);
    BlackLibraryCLI &operator = (BlackLibraryCLI &&) = default;

    int Run();
    int Stop();

private:
    void ProcessInput(const std::string &input);

    black_library::core::db::BlackLibraryDB blacklibrary_db_;
    std::string storage_path_;
    std::atomic_bool done_;
};

} // namespace black_library

#endif
