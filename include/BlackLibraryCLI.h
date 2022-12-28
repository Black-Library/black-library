/**
 * BlackLibraryCLI.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_CLI_H__

#include <atomic>
#include <vector>

#include <ConfigOperations.h>
#include <BlackLibraryBinder.h>
#include <BlackLibraryDB.h>

namespace black_library {

namespace BlackLibraryBinder = black_library::core::binder;
namespace BlackLibraryDB = black_library::core::db;

class BlackLibraryCLI {
public:
    explicit BlackLibraryCLI(const njson &config);
    BlackLibraryCLI &operator = (BlackLibraryCLI &&) = default;

    int Run();
    int Stop();

private:
    void BindEntry(const std::vector<std::string> &tokens);
    void ChangeSize(const std::vector<std::string> &tokens);
    void ChangeSizeAll(const std::vector<std::string> &tokens);
    void DeleteEntry(const std::vector<std::string> &tokens);
    void Export(const std::vector<std::string> &tokens);
    void ExportChecksums(const std::vector<std::string> &tokens);
    void ExportEntries(const std::vector<std::string> &tokens, const std::string &type);
    void Import(const std::vector<std::string> &tokens);
    void ImportChecksums(const std::vector<std::string> &tokens);
    void ImportEntries(const std::vector<std::string> &tokens, const std::string &type);
    void List(const std::vector<std::string> &tokens);
    void ListChecksums(const std::vector<std::string> &tokens);
    void ListEntries(const std::vector<std::string> &tokens, const std::string &type);
    void PrintUsage(const std::vector<std::string> &tokens);
    void VersionAll(const std::vector<std::string> &tokens);

    void ProcessInput(const std::vector<std::string> &tokens);

    void SanatizeInput(std::vector<std::string> &tokens);

    BlackLibraryDB::BlackLibraryDB blacklibrary_db_;
    BlackLibraryBinder::BlackLibraryBinder blacklibrary_binder_;
    std::string logger_name_;
    std::atomic_bool done_;
};

} // namespace black_library

#endif
