/**
 * BlackLibraryCLI.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__

#include <atomic>
#include <vector>

#include <BlackLibraryBinder.h>
#include <BlackLibraryDB.h>

namespace black_library {

class BlackLibraryCLI {
public:
    explicit BlackLibraryCLI(const std::string &db_path, const std::string &storage_path);
    BlackLibraryCLI &operator = (BlackLibraryCLI &&) = default;

    int Run();
    int Stop();

private:
    void BindEntry(const std::vector<std::string> &tokens);
    void PrintEntries(const std::vector<std::string> &tokens);
    void PrintUsage(const std::vector<std::string> &tokens);
    void ProcessInput(const std::vector<std::string> &tokens);

    black_library::core::db::BlackLibraryDB blacklibrary_db_;
    black_library::core::binder::BlackLibraryBinder blacklibrary_binder_;
    std::atomic_bool done_;
};

} // namespace black_library

#endif