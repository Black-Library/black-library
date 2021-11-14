/**
 * BlackLibraryCLI.cc
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <StringOperations.h>

#include <BlackLibraryCLI.h>

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

static constexpr const char DefaultPrintEntryFileName[] = "black_library_print_entries";

template <typename T>
inline size_t DBColumnIDCast(T const &id)
{
    return static_cast<size_t>(id);
}

BlackLibraryCLI::BlackLibraryCLI(const std::string &db_path, const std::string &storage_path) :
    blacklibrary_db_(db_path, false),
    blacklibrary_binder_(storage_path),
    done_(false)
{
    BlackLibraryCommon::InitRotatingLogger("black_library_cli", "/mnt/black-library/log/", true);
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
        BlackLibraryCommon::LogError("black_library_cli", "Black entry with UUID: {} does not exist for bind", target_uuid);
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
        BlackLibraryCommon::LogError("black_library_cli", "Black entry with UUID: {} does not exist for size", target_uuid);
        return;
    }

    auto black_entry = blacklibrary_db_.ReadBlackEntry(target_uuid);

    black_entry.series_length = desired_size;
    black_entry.last_url = black_entry.url;

    if (blacklibrary_db_.UpdateBlackEntry(black_entry))
    {
        BlackLibraryCommon::LogError("black_library_cli", "Failed to update black entry with UUID: {} size: {}", target_uuid, desired_size);
        return;
    }

    BlackLibraryCommon::LogInfo("black_library_cli", "Changed size of UUID: {} to {}", target_uuid, desired_size);

    if (!blacklibrary_db_.DoesStagingEntryUUIDExist(target_uuid))
    {
        BlackLibraryCommon::LogWarn("black_library_cli", "Staging UUID: {} does not exist for size", target_uuid);
        return;
    }

    auto staging_entry = blacklibrary_db_.ReadStagingEntry(target_uuid);

    staging_entry.series_length = desired_size;
    staging_entry.last_url = staging_entry.url;

    if (blacklibrary_db_.UpdateStagingEntry(staging_entry))
    {
        BlackLibraryCommon::LogError("black_library_cli", "Failed to update staging entry with UUID: {} size: {}", target_uuid, desired_size);
        return;
    }
}

void BlackLibraryCLI::ChangeSizeAll(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::string target_source;
    size_t desired_size;
    time_t desired_after_date;

    if (tokens.size() >= 3)
    {
        desired_size = std::stol(tokens[1]);
        desired_after_date = std::stol(tokens[2]);
        std::cout << desired_after_date << std::endl;
    }
    else
    {
        std::cout << "sizeall [new_size] [seconds since epoch desired after date]" << std::endl;
        return;
    }

    if (tokens.size() >= 4)
    {
        target_source = tokens[3];
    }

    entry_list = blacklibrary_db_.GetBlackEntryList();

    for (auto &entry : entry_list)
    {
        // std::cout << entry.uuid << " update date: " << entry.update_date << " after_date: " << desired_after_date << " - " << (entry.update_date > desired_after_date) << " - " << desired_size << std::endl;
        // change size if they come after the provided date
        if (entry.update_date > desired_after_date && entry.source == target_source)
        {
            entry.series_length = desired_size;
            entry.last_url = entry.url;

            if (blacklibrary_db_.UpdateBlackEntry(entry))
            {
                BlackLibraryCommon::LogError("black_library_cli", "Failed to update black entry with UUID: {} size: {}", entry.uuid, desired_size);
                continue;
            }

            BlackLibraryCommon::LogInfo("black_library_cli", "Changed size of UUID: {} to {}", entry.uuid, desired_size);
        }
    }
}

void BlackLibraryCLI::DeleteEntry(const std::vector<std::string> &tokens)
{
    std::string target_entry_type;
    std::string target_uuid;

    if (tokens.size() >= 2)
    {
        target_uuid = tokens[1];
    }
    else
    {
        std::cout << "delete [uuid] (table)" << std::endl;
        return;
    }

    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[2];
    }

    if (target_entry_type == "black")
    {
        if (!blacklibrary_db_.DoesBlackEntryUUIDExist(target_uuid))
        {
            BlackLibraryCommon::LogError("black_library_cli", "Black entry with UUID: {} does not exist for delete", target_uuid);
            return;
        }

        if (blacklibrary_db_.DeleteBlackEntry(target_uuid))
        {
            BlackLibraryCommon::LogError("black_library_cli", "Failed to delete black entry with UUID: {}", target_uuid);
            return;
        }
    }
    else if (target_entry_type == "staging")
    {
        if (!blacklibrary_db_.DoesStagingEntryUUIDExist(target_uuid))
        {
            BlackLibraryCommon::LogError("black_library_cli", "Staging entry with UUID: {} does not exist for delete", target_uuid);
            return;
        }

        if (blacklibrary_db_.DeleteStagingEntry(target_uuid))
        {
            BlackLibraryCommon::LogError("black_library_cli", "Failed to delete staging entry with UUID: {}", target_uuid);
            return;
        }
    }
    else
    {
        BlackLibraryCommon::LogWarn("black_library_cli", "Failed to match entry with UUID: {}", target_uuid);
    }

}

void BlackLibraryCLI::ListEntries(const std::vector<std::string> &tokens)
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
    else if (target_entry_type == "error")
        error_list = blacklibrary_db_.GetErrorEntryList();
    else if (target_entry_type == "staging")
        entry_list = blacklibrary_db_.GetStagingEntryList();
    else if (target_entry_type == "help")
        std::cout << "list [black, error, staging]" << std::endl;
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

void BlackLibraryCLI::PrintEntries(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::vector<BlackLibraryDB::ErrorEntry> error_list;
    std::string target_entry_type;
    std::string target_path = DefaultPrintEntryFileName;
    std::stringstream ss;

    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[1];
    }

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    if (target_entry_type == "black")
        entry_list = blacklibrary_db_.GetBlackEntryList();
    else if (target_entry_type == "error")
        error_list = blacklibrary_db_.GetErrorEntryList();
    else if (target_entry_type == "staging")
        entry_list = blacklibrary_db_.GetStagingEntryList();
    else if (target_entry_type == "help")
        std::cout << "save [black, error, staging]" << std::endl;
    else
    {
        auto staging_entries = blacklibrary_db_.GetStagingEntryList();
        auto black_entires = blacklibrary_db_.GetBlackEntryList();
        entry_list.reserve(staging_entries.size() + black_entires.size());
        entry_list.insert(entry_list.end(), staging_entries.begin(), staging_entries.end());
        entry_list.insert(entry_list.end(), black_entires.begin(), black_entires.end());

        error_list = blacklibrary_db_.GetErrorEntryList();
    }

    for (const auto & entry : entry_list)
    {
        ss << entry.uuid << ',';
        ss << entry.title << ',';
        ss << entry.author << ',';
        ss << entry.nickname << ',';
        ss << entry.source << ',';
        ss << entry.url << ',';
        ss << entry.last_url << ',';
        ss << entry.series << ',';
        ss << entry.series_length << ',';
        ss << entry.version << ',';
        ss << entry.media_path << ',';
        ss << entry.birth_date << ',';
        ss << entry.check_date << ',';
        ss << entry.update_date << ',';
        ss << entry.user_contributed;
        ss << '\n';
    }

    std::fstream output_file;

    if (BlackLibraryCommon::FileExists(target_path))
    {
        BlackLibraryCommon::LogError("black_library_cli", "File already exists for print");
        return;
    }

    output_file.open(target_path, std::fstream::out | std::fstream::trunc);

    if (!output_file.is_open())
    {
        BlackLibraryCommon::LogError("black_library_cli", "Failed to open file with path: {} for print", target_path);
        return;
    }

    output_file << ss.str();

    output_file.close();
}

void BlackLibraryCLI::PrintUsage(const std::vector<std::string> &tokens)
{
    std::stringstream ss;

    ss << "Usage: [bind, delete, help, list, print (to file), save (from file), size, sizeall]";

    // TODO make some kind of command mapping/register

    std::cout << ss.str() << std::endl;
    std::cout << "Input: " << std::endl; 

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << tokens[i] << ' ';
    }

    std::cout << std::endl;
}

void BlackLibraryCLI::SaveEntries(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::string target_entry_type;
    std::string target_path = DefaultPrintEntryFileName;

    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[1];   
    }
    else
    {
        std::cout << "save [type] [path]" << std::endl;
        return;
    }

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    if (!BlackLibraryCommon::FileExists(target_path))
    {
        BlackLibraryCommon::LogError("black_library_cli", "File does not exist with path: {} for save", target_path);
        std::cout << target_path << " file does not exist" << std::endl;
        return;
    }

    std::ifstream input_file;
    std::string input_file_line;
    std::vector<std::string> entry_lines;

    input_file.open(target_path, std::fstream::in);

    if (!input_file.is_open())
    {
        BlackLibraryCommon::LogError("black_library_cli", "Failed to open file with path: {} for save", target_path);
        return;
    }

    while (getline(input_file, input_file_line))
    {
        entry_lines.emplace_back(input_file_line);
    }

    for (const auto & entry_line : entry_lines)
    {
        std::istringstream is(entry_line);
        std::vector<std::string> tokens;
        std::string token;
        while (getline(is, token, ','))
        {
            tokens.emplace_back(token);
        }

        if (tokens.size() < static_cast<size_t>(BlackLibraryDB::DBEntryColumnID::_NUM_DB_ENTRY_COLUMN_ID))
        {
            BlackLibraryCommon::LogWarn("black_library_cli", "Failed to read: {}", entry_line);
            continue;
        }

        BlackLibraryDB::DBEntry entry = {
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::uuid)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::title)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::author)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::nickname)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::source)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::url)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::last_url)],
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::series)],
            static_cast<uint16_t>(stoul(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::series_length)])),
            static_cast<uint16_t>(stoul(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::version)])),
            tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::media_path)],
            stol(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::birth_date)]),
            stol(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::check_date)]),
            stol(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::update_date)]),
            static_cast<uint16_t>(stoul(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::user_contributed)]))
        };

        if (blacklibrary_db_.DoesBlackEntryUUIDExist(entry.uuid))
        {
            blacklibrary_db_.UpdateBlackEntry(entry);
        }
        else
        {
            blacklibrary_db_.CreateBlackEntry(entry);
        }
    }
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
    else if (command == "delete")
    {
        DeleteEntry(tokens);
    }
    else if (command == "help")
    {
        PrintUsage(tokens);
    }
    else if (command == "list")
    {
        ListEntries(tokens);
    }
    else if (command == "print")
    {
        PrintEntries(tokens);
    }
    else if (command == "save")
    {
        SaveEntries(tokens);
    }
    else if (command == "size")
    {
        ChangeSize(tokens);
    }
    else if (command == "sizeall")
    {
        ChangeSizeAll(tokens);
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
