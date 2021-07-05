/**
 * BlackLibraryCLI.cc
 */

#include <iostream>
#include <sstream>

#include <FileOperations.h>
#include <StringOperations.h>

#include <BlackLibraryCLI.h>

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

BlackLibraryCLI::BlackLibraryCLI(const std::string &db_path, const std::string &storage_path) :
    blacklibrary_db_(db_path, false),
    blacklibrary_binder_(storage_path),
    done_(false)
{

}

int BlackLibraryCLI::Run()
{
    while (!done_)
    {
        std::vector<std::string> tokens;
        std::stringstream ss;
        std::string input;
        std::string intermediate;
        std::cout << "BlackLibraryCLI>";

        std::getline(std::cin, input);

        ss << input;

        while (getline(ss, intermediate, ' '))
        {
            tokens.push_back(intermediate);
        }

        SanatizeInput(tokens);

        ProcessInput(tokens);
    }

    return 0;
}

int BlackLibraryCLI::Stop()
{
    done_ = true;

    return 0;
}

void BlackLibraryCLI::BindEntry(const std::vector<std::string> &tokens)
{
    std::string target_uuid;
    if (tokens.size() >= 2)
    {
        target_uuid = tokens[1];
    }
    else
    {
        std::cout << "bind [uuid]" << std::endl;
        return;
    }

    if (!blacklibrary_db_.DoesBlackEntryUUIDExist(target_uuid))
    {
        std::cout << "Error: uuid: " << target_uuid << " does not exist" << std::endl;
        return;
    }

    auto entry = blacklibrary_db_.ReadBlackEntry(target_uuid);

    blacklibrary_binder_.Bind(target_uuid, entry.title);
}

void BlackLibraryCLI::ChangeSize(const std::vector<std::string> &tokens)
{
    std::string target_uuid;
    size_t desired_size;
    if (tokens.size() >= 3)
    {
        target_uuid = tokens[1];
        desired_size = std::stol(tokens[2]);
    }
    else
    {
        std::cout << "size [uuid] [new_size]" << std::endl;
        return;
    }

    if (!blacklibrary_db_.DoesBlackEntryUUIDExist(target_uuid))
    {
        std::cout << "Error: uuid: " << target_uuid << " does not exist" << std::endl;
        return;
    }

    auto entry = blacklibrary_db_.ReadBlackEntry(target_uuid);

    entry.series_length = desired_size;
    entry.last_url = entry.url;

    blacklibrary_db_.UpdateBlackEntry(entry);
}

void BlackLibraryCLI::PrintEntries(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::vector<BlackLibraryDB::ErrorEntry> error_list;
    std::string target_entry_type;
    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[1];
    }

    if (target_entry_type == "black")
        entry_list = blacklibrary_db_.GetBlackEntryList();
    if (target_entry_type == "error")
        error_list = blacklibrary_db_.GetErrorEntryList();
    else if (target_entry_type == "staging")
        entry_list = blacklibrary_db_.GetStagingEntryList();
    else if (target_entry_type == "help")
        std::cout << "print [black, error, staging]" << std::endl;
    else
    {
        auto staging_entries = blacklibrary_db_.GetStagingEntryList();
        auto black_entires = blacklibrary_db_.GetBlackEntryList();
        entry_list.reserve(staging_entries.size() + black_entires.size());
        entry_list.insert(entry_list.end(), staging_entries.begin(), staging_entries.end());
        entry_list.insert(entry_list.end(), black_entires.begin(), black_entires.end());

        error_list = blacklibrary_db_.GetErrorEntryList();
    }
    
    std::cout << "Entries" << std::endl;

    for (const auto & entry : entry_list)
    {
        std::cout << entry << std::endl;
    }

    std::cout << "Errors" << std::endl;

    for (const auto & entry : error_list)
    {
        std::cout << entry << std::endl;
    }
}

void BlackLibraryCLI::PrintUsage(const std::vector<std::string> &tokens)
{
    std::stringstream ss;

    ss << "Usage: [bind, help, list, size]";

    // TODO make some kind of command mapping/register

    std::cout << ss.str() << std::endl;
    std::cout << "Input: " << std::endl; 

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << tokens[i] << ' ';
    }

    std::cout << std::endl;
}

void BlackLibraryCLI::ProcessInput(const std::vector<std::string> &tokens)
{
    std::string command = tokens[0];
    if (command == "stop" || command == "quit" || command == "exit")
    {
        Stop();
    }
    else if (command == "bind")
    {
        BindEntry(tokens);
    }
    else if (command == "help")
    {
        PrintUsage(tokens);
    }
    else if (command == "list")
    {
        PrintEntries(tokens);
    }
    else if (command == "size")
    {
        ChangeSize(tokens);
    }
    else
    {
        PrintUsage(tokens);
    }
    std::cout << "COMMAND: " << command << std::endl;
}

void BlackLibraryCLI::SanatizeInput(std::vector<std::string> &tokens)
{
    for (auto & token : tokens)
    {
        BlackLibraryCommon::SanatizeString(token);
    }
}

} // namespace black_library
