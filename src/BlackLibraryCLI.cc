/**
 * BlackLibraryCLI.cc
 */

#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <SourceInformation.h>
#include <StringOperations.h>
#include <VersionOperations.h>

#include <BlackLibraryCLI.h>

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

static constexpr const char DefaultExportEntryFileName[] = "black_library_export_entries";
static constexpr const char DefaultExportChecksumFileName[] = "black_library_export_checksums";

template <typename T>
inline size_t DBColumnIDCast(T const &id)
{
    return static_cast<size_t>(id);
}

BlackLibraryCLI::BlackLibraryCLI(const njson &config) :
    blacklibrary_db_(config),
    blacklibrary_binder_(config),
    logger_name_("black_library_cli"),
    done_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    BlackLibraryCommon::InitRotatingLogger(logger_name_, logger_path, true);
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

    if (!blacklibrary_db_.DoesWorkEntryUUIDExist(target_uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Black entry with UUID: {} does not exist for bind", target_uuid);
        return;
    }

    auto entry = blacklibrary_db_.ReadWorkEntry(target_uuid);

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

    if (!blacklibrary_db_.DoesWorkEntryUUIDExist(target_uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Black entry with UUID: {} does not exist for size", target_uuid);
        return;
    }

    auto work_entry = blacklibrary_db_.ReadWorkEntry(target_uuid);

    work_entry.series_length = desired_size;
    work_entry.last_url = work_entry.url;

    if (blacklibrary_db_.UpdateWorkEntry(work_entry))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to update black entry with UUID: {} size: {}", target_uuid, desired_size);
        return;
    }

    BlackLibraryCommon::LogInfo(logger_name_, "Changed size of UUID: {} to {}", target_uuid, desired_size);
}

void BlackLibraryCLI::ChangeSizeAll(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> work_entry_list;
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
        std::cout << "sizeall [new_size] [seconds since epoch desired after date] [source]" << std::endl;
        return;
    }

    if (tokens.size() >= 4)
    {
        target_source = tokens[3];
    }

    work_entry_list = blacklibrary_db_.GetWorkEntryList();

    for (auto &entry : work_entry_list)
    {
        // std::cout << entry.uuid << " update date: " << entry.update_date << " after_date: " << desired_after_date << " - " << (entry.update_date > desired_after_date) << " - " << desired_size << std::endl;
        // change size if they come after the provided date
        if (entry.update_date > desired_after_date && entry.source == target_source)
        {
            entry.series_length = desired_size;
            entry.last_url = entry.url;

            if (blacklibrary_db_.UpdateWorkEntry(entry))
            {
                BlackLibraryCommon::LogError(logger_name_, "Failed to update black entry with UUID: {} size: {}", entry.uuid, desired_size);
                continue;
            }

            BlackLibraryCommon::LogInfo(logger_name_, "Changed size of UUID: {} to {}", entry.uuid, desired_size);
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
        std::cout << "delete [uuid]" << std::endl;
        return;
    }

    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[2];
    }

    if (!blacklibrary_db_.DoesWorkEntryUUIDExist(target_uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Entry with UUID: {} does not exist for delete", target_uuid);
        return;
    }

    if (blacklibrary_db_.DeleteWorkEntry(target_uuid))
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to delete entry with UUID: {}", target_uuid);
        return;
    }
}

void BlackLibraryCLI::Export(const std::vector<std::string> &tokens)
{
    std::string target_type;

    if (tokens.size() >= 2)
    {
        target_type = tokens[1];
    }

    if (target_type == "work")
        ExportEntries(tokens, "work");
    else if (target_type == "checksum")
        ExportChecksums(tokens);
    else if (target_type == "error")
        ExportEntries(tokens, "error");
    else if (target_type == "all")
    {
        ExportEntries(tokens, "work");
        ExportChecksums(tokens);
    }
    
    else
        std::cout << "export [checksum, error, work]" << std::endl;
}

void BlackLibraryCLI::ExportChecksums(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryCommon::Md5Sum> checksum_list;
    std::string target_path = DefaultExportChecksumFileName;
    std::stringstream ss;

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    checksum_list = blacklibrary_db_.GetChecksumList();

    for (const auto & checksum : checksum_list)
    {
        ss << checksum.uuid << ',';
        ss << checksum.index_num << ',';
        ss << checksum.md5_sum << ',';
        ss << checksum.date << ',';
        ss << checksum.sec_id << ',';
        ss << checksum.seq_num << ',';
        ss << checksum.version_num;
        ss << '\n';
    }

    std::fstream output_file;

    if (BlackLibraryCommon::PathExists(target_path))
    {
        BlackLibraryCommon::LogError(logger_name_, "File already exists for export");
        return;
    }

    output_file.open(target_path, std::fstream::out | std::fstream::trunc);

    if (!output_file.is_open())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to open file with path: {} for export", target_path);
        return;
    }

    output_file << ss.str();

    output_file.close();
}

void BlackLibraryCLI::ExportEntries(const std::vector<std::string> &tokens, const std::string &type)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::vector<BlackLibraryDB::DBErrorEntry> error_list;
    std::string target_path = DefaultExportEntryFileName;
    std::stringstream ss;

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    if (type == "work")
        entry_list = blacklibrary_db_.GetWorkEntryList();
    else if (type == "error")
        error_list = blacklibrary_db_.GetErrorEntryList();
    else if (type == "help")
        std::cout << "export [error, work] path" << std::endl;
    else
    {
        auto work_entries = blacklibrary_db_.GetWorkEntryList();
        entry_list.reserve(work_entries.size());
        entry_list.insert(entry_list.end(), work_entries.begin(), work_entries.end());

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
        ss << entry.user_contributed << ',';
        ss << entry.processing;
        ss << '\n';
    }

    std::fstream output_file;

    if (BlackLibraryCommon::PathExists(target_path))
    {
        BlackLibraryCommon::LogError(logger_name_, "File already exists for export");
        return;
    }

    output_file.open(target_path, std::fstream::out | std::fstream::trunc);

    if (!output_file.is_open())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to open file with path: {} for export", target_path);
        return;
    }

    output_file << ss.str();

    output_file.close();
}

void BlackLibraryCLI::Import(const std::vector<std::string> &tokens)
{
    std::string target_type;

    if (tokens.size() >= 2)
    {
        target_type = tokens[1];
    }

    if (target_type == "work")
        ImportEntries(tokens);
    else if (target_type == "checksum")
        ImportChecksums(tokens);
    else if (target_type == "all")
    {
        ImportEntries(tokens);
        ImportChecksums(tokens);
    }
    
    else
        std::cout << "import [all, checksum, error, work] path" << std::endl;
}

void BlackLibraryCLI::ImportChecksums(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryCommon::Md5Sum> checksum_list;
    std::string target_path = DefaultExportChecksumFileName;

    BlackLibraryCommon::LogDebug(logger_name_, "Start ImportChecksums");

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    if (!BlackLibraryCommon::PathExists(target_path))
    {
        BlackLibraryCommon::LogError(logger_name_, "File does not exist with path: {} for import", target_path);
        return;
    }

    std::ifstream input_file;
    std::string input_file_line;
    std::vector<std::string> checksum_lines;

    input_file.open(target_path, std::fstream::in);

    if (!input_file.is_open())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to open file with path: {} for import", target_path);
        return;
    }

    while (getline(input_file, input_file_line))
    {
        checksum_lines.emplace_back(input_file_line);
    }

    input_file.close();

    for (const auto & checksum_line : checksum_lines)
    {
        std::istringstream is(checksum_line);
        std::vector<std::string> tokens;
        std::string token;
        while (getline(is, token, ','))
        {
            tokens.emplace_back(token);
        }

        if (tokens.size() < static_cast<size_t>(BlackLibraryCommon::DBMd5SumColumnID::_NUM_DB_MD5SUM_COLUMN_ID))
        {
            BlackLibraryCommon::LogWarn(logger_name_, "Failed to read md5sum: {}, size", checksum_line);
            continue;
        }

        try
        {
            BlackLibraryCommon::Md5Sum checksum = {
                tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::uuid)],
                tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::md5_sum)],
                static_cast<size_t>(stoul(tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::index_num)])),
                stol(tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::date)]),
                tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::sec_id)],
                static_cast<BlackLibraryCommon::seq_num_rep_t>(stoul(tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::seq_num)])),
                static_cast<uint16_t>(stoul(tokens[DBColumnIDCast(BlackLibraryCommon::DBMd5SumColumnID::version_num)])),
            };

            if (blacklibrary_db_.DoesMd5SumExistByIndexNum(checksum.uuid, checksum.index_num))
            {
                if (blacklibrary_db_.UpdateMd5SumBySeqNum(checksum))
                {
                    BlackLibraryCommon::LogError(logger_name_, "ImportChecksums UpdateMd5SumBySeqNum failure: {} - {} - {}", checksum.uuid, checksum.index_num, checksum.md5_sum);
                    break;
                }
            }
            else
            {
                if(blacklibrary_db_.CreateMd5Sum(checksum))
                {
                    BlackLibraryCommon::LogError(logger_name_, "ImportChecksums CreateMd5Sum failure: {} - {} - {}", checksum.uuid, checksum.index_num, checksum.md5_sum);
                    break;
                }
            }
        }
        catch(const std::invalid_argument& ex)
        {
            BlackLibraryCommon::LogError(logger_name_, "ImportChecksums error: {}", std::string(ex.what()));
            for (const auto & token : tokens)
            {
                BlackLibraryCommon::LogError(logger_name_, "ImportChecksums error: {}", token);
            }
            return;
        }
    }
}

void BlackLibraryCLI::ImportEntries(const std::vector<std::string> &tokens)
{
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::string target_path = DefaultExportEntryFileName;

    BlackLibraryCommon::LogDebug(logger_name_, "Start ImportEntries");

    if (tokens.size() >= 3)
    {
        target_path = tokens[2];
    }

    if (!BlackLibraryCommon::PathExists(target_path))
    {
        BlackLibraryCommon::LogError(logger_name_, "File does not exist with path: {} for import", target_path);
        return;
    }

    std::ifstream input_file;
    std::string input_file_line;
    std::vector<std::string> entry_lines;

    input_file.open(target_path, std::fstream::in);

    if (!input_file.is_open())
    {
        BlackLibraryCommon::LogError(logger_name_, "Failed to open file with path: {} for import", target_path);
        return;
    }

    while (getline(input_file, input_file_line))
    {
        entry_lines.emplace_back(input_file_line);
    }

    input_file.close();

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
            BlackLibraryCommon::LogWarn(logger_name_, "Failed to read entry: {}, size", entry_line);
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
            static_cast<uint16_t>(stoul(tokens[DBColumnIDCast(BlackLibraryDB::DBEntryColumnID::user_contributed)])),
            false,
        };

        if (blacklibrary_db_.DoesWorkEntryUUIDExist(entry.uuid))
        {
            blacklibrary_db_.UpdateWorkEntry(entry);
        }
        else
        {
            blacklibrary_db_.CreateWorkEntry(entry);
        }
    }
}

void BlackLibraryCLI::List(const std::vector<std::string> &tokens)
{
    std::string target_entry_type;

    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[1];
    }

    if (target_entry_type == "work")
        ListEntries(tokens, "work");
    else if (target_entry_type == "checksum")
        ListChecksums(tokens);
    else if (target_entry_type == "error")
        ListEntries(tokens, "error");
    else
        std::cout << "list [work, error, checksum]" << std::endl;
}

void BlackLibraryCLI::ListChecksums(const std::vector<std::string> &tokens)
{
    std::string target_uuid = "";
    auto checksum_list = blacklibrary_db_.GetChecksumList();

    if (tokens.size() >= 3)
    {
        target_uuid = tokens[2];
    }

    if (target_uuid.empty())
    {
        for (const auto & checksum : checksum_list)
        {
            std::cout << checksum << std::endl;
        }
    }
    else
    {
        for (const auto & checksum : checksum_list)
        {
            if (checksum.uuid == target_uuid)
                std::cout << checksum << std::endl;
        }
    }
}

void BlackLibraryCLI::ListEntries(const std::vector<std::string> &tokens, const std::string &type)
{
    std::string target_uuid = "";
    std::vector<BlackLibraryDB::DBEntry> entry_list;
    std::vector<BlackLibraryDB::DBErrorEntry> error_list;

    if (tokens.size() >= 3)
    {
        target_uuid = tokens[2];
    }

    if (type == "work")
        entry_list = blacklibrary_db_.GetWorkEntryList();
    else if (type == "error")
        error_list = blacklibrary_db_.GetErrorEntryList();
    else if (type == "help")
        std::cout << "list [error, work] (uuid)" << std::endl;
    else
    {
        auto work_entries = blacklibrary_db_.GetWorkEntryList();
        entry_list.reserve(work_entries.size());
        entry_list.insert(entry_list.end(), work_entries.begin(), work_entries.end());

        error_list = blacklibrary_db_.GetErrorEntryList();
    }

    std::cout << "Entries" << std::endl;

    if (target_uuid.empty())
    {
        for (const auto & entry : entry_list)
        {
            std::cout << entry << std::endl;
        }
    }
    else
    {
        for (const auto & entry : entry_list)
        {
            if (entry.uuid == target_uuid)
                std::cout << entry << std::endl;
        }
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

    ss << "Usage: [bind, delete, export, help, import, list, reorder, size, sizeall, squash, updateidentifier, versionall]";

    // TODO make some kind of command mapping/register

    std::cout << ss.str() << std::endl;
    std::cout << "Input: " << std::endl; 

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << tokens[i] << ' ';
    }

    std::cout << std::endl;
}

void BlackLibraryCLI::SquashMd5Indexes(const std::vector<std::string> &tokens)
{
    (void) tokens;

    std::vector<BlackLibraryCommon::Md5Sum> md5_list;

    md5_list = blacklibrary_db_.GetChecksumList();

    // organize by uuid
    std::unordered_map<std::string, std::vector<BlackLibraryCommon::Md5Sum>> md5_map;

    for (const auto &md5 : md5_list)
    {
        auto it = md5_map.find(md5.uuid);
        if (it == md5_map.end())
        {
            std::vector<BlackLibraryCommon::Md5Sum> md5_sublist;
            md5_sublist.emplace_back(md5);
            md5_map.emplace(md5.uuid, md5_sublist);
        }
        else
        {
            std::vector<BlackLibraryCommon::Md5Sum> md5_sublist = it->second;
            md5_sublist.emplace_back(md5);
            it->second = md5_sublist;
        }
    }

    for (auto &ele : md5_map)
    {
        std::sort(ele.second.begin(), ele.second.end());
        std::vector<BlackLibraryCommon::Md5Sum> md5_sublist = ele.second;
        BlackLibraryCommon::Md5Sum last = md5_sublist.back();
        // only works with no version increment, increments would increase size of md5 vector
        if (last.index_num + 1 != ele.second.size())
        {
            std::cout << "Found " << ele.second.size() << " entries for " << ele.first << std::endl;
            std::cout << "size mismatch " << last.index_num + 1 << " - " << ele.second.size() << std::endl;

            size_t expected_index = 0;
            for (auto & md5 : md5_sublist){
                if (md5.index_num != expected_index){
                    std::cout << "setting md5 index: " << md5.index_num << " to " << expected_index << std::endl;
                    if (blacklibrary_db_.DeleteMd5Sum(md5.uuid, md5.index_num))
                    {
                        std::cout << "error, delete md5 sum failed" << std::endl;
                        return;
                    }
                    md5.index_num = expected_index;
                    if (blacklibrary_db_.CreateMd5Sum(md5))
                    {
                        std::cout << "error, create md5 sum failed" << std::endl;
                        return;
                    }
                }
                ++expected_index;
            }
        }
    }
}

void BlackLibraryCLI::ReorderMd5(const std::vector<std::string> &tokens)
{
    BlackLibraryCommon::LogDebug(logger_name_, "Start ReorderMd5");

    std::string reorder_target = BlackLibraryCommon::RR::source_name;
    std::string exact_target = "";

    if (tokens.size() >= 2)
    {
        reorder_target = tokens[1];
    }

    if (tokens.size() >= 3)
    {
        exact_target = tokens[2];
    }

    if (!exact_target.empty())
    {
        BlackLibraryCommon::LogDebug(logger_name_, "exact_target: {}", exact_target);
    }

    BlackLibraryCommon::LogDebug(logger_name_, "Reordering {}", BlackLibraryCommon::RR::source_name);

    std::vector<BlackLibraryDB::DBEntry> work_entries = blacklibrary_db_.GetWorkEntryList();
    std::vector<BlackLibraryDB::DBEntry> target_entries;

    for (const auto & work_entry : work_entries)
    {
        if (work_entry.source == reorder_target)
        {
            if (exact_target.empty())
            {
                target_entries.emplace_back(work_entry);
            }
            else
            {
                if (work_entry.uuid != exact_target)
                    continue;
                target_entries.emplace_back(work_entry);
            }
        }
    }

    BlackLibraryCommon::LogDebug(logger_name_, "Targeting {} work entries", target_entries.size());

    for (const auto & target_entry : target_entries)
    {
        std::priority_queue<BlackLibraryCommon::Md5Sum, std::vector<BlackLibraryCommon::Md5Sum>, BlackLibraryCommon::Md5SumGreaterThanBySeqNum> md5_seq_num_queue;
        auto md5s = blacklibrary_db_.GetMd5SumsFromUUID(target_entry.uuid);

        // BlackLibraryCommon::LogDebug(logger_name_, "UUID: {} found {} md5s", target_entry.uuid, md5s.size());

        size_t max_seq_count = 0;
        for (const auto & md5 : md5s)
        {

            if (md5.seq_num == BlackLibraryCommon::MaxSeqNum)
            {
                ++max_seq_count;
                continue;
            }
            md5_seq_num_queue.push(md5);
        }
        if (max_seq_count > 0)
            BlackLibraryCommon::LogDebug(logger_name_, "UUID: {} found {} max seq instances", target_entry.uuid, max_seq_count);

        // size_t expected_index = 0;
        // while(!md5_seq_num_queue.empty())
        // {
        //     BlackLibraryCommon::Md5Sum md5_update = md5_seq_num_queue.top();
        //     // BlackLibraryCommon::LogDebug(logger_name_, "seq: {}", md5_update.seq_num);
        //     if (md5_update.index_num != expected_index)
        //     {
        //         BlackLibraryCommon::LogDebug(logger_name_, "index: {} - expected index: {}", md5_update.index_num, expected_index);
        //         md5_update.index_num = expected_index;
        //         if (blacklibrary_db_.UpdateMd5SumBySeqNum(md5_update))
        //         {
        //             BlackLibraryCommon::LogError(logger_name_, "Failed update md5 sum by seq num");
        //             break;
        //         }
        //     }
        //     ++expected_index;
        //     md5_seq_num_queue.pop();
        // }
    }
}


void BlackLibraryCLI::UpdateMd5Identifier(const std::vector<std::string> &tokens)
{
    (void) tokens;
    auto checksums = blacklibrary_db_.GetChecksumList();

    size_t modify_count = 0;
    for (auto & checksum : checksums) 
    {
        if (BlackLibraryCommon::ContainsString(checksum.sec_id, "https://www.royalroad.com") || BlackLibraryCommon::ContainsString(checksum.sec_id, "forums.spacebattles") || BlackLibraryCommon::ContainsString(checksum.sec_id, "forums.sufficientvelocity"))
        {
            std::string old_sec_id = checksum.sec_id;
            std::string updated_sec_id = BlackLibraryCommon::GetWorkChapterSecIdFromUrl(checksum.sec_id);
            size_t updated_seq_num = BlackLibraryCommon::GetWorkChapterSeqNumFromUrl(checksum.sec_id);
            checksum.sec_id = updated_sec_id;
            checksum.seq_num = updated_seq_num;
            BlackLibraryCommon::LogTrace(logger_name_, "change {} to {} and {}", old_sec_id, checksum.sec_id, checksum.seq_num);
            blacklibrary_db_.UpdateMd5SumByIndexNum(checksum);
            ++modify_count;
        }

        // for updating md5 sum with page-1 from messed up tests
        // if (!(checksum.sec_id.empty() && checksum.seq_num < BlackLibraryCommon::MaxSeqNum))
        //     continue;
        // if (!blacklibrary_db_.DoesWorkEntryUUIDExist(checksum.uuid))
        //     continue;

        // BlackLibraryDB::DBEntry work_entry = blacklibrary_db_.ReadWorkEntry(checksum.uuid);
        // if (!BlackLibraryCommon::ContainsString(work_entry.url, "forums."))
        //     continue;

        // checksum.sec_id = "page-1";
        // blacklibrary_db_.UpdateMd5SumByIndexNum(checksum);
        // ++modify_count;
    }

    BlackLibraryCommon::LogInfo(logger_name_, "Modified {} checksum rows", modify_count);
}

void BlackLibraryCLI::VersionAll(const std::vector<std::string> &tokens)
{
    (void) tokens;
    std::vector<BlackLibraryDB::DBEntry> entry_list;

    entry_list = blacklibrary_db_.GetWorkEntryList();

    for (auto &entry : entry_list)
    {
        // series_length is uint16_t
        for (uint16_t i = 0; i < entry.series_length - 1; ++i)
        {
            if (blacklibrary_db_.DoesMd5SumExistByIndexNum(entry.uuid, i))
                continue;

            BlackLibraryCommon::Md5Sum checksum;

            checksum.uuid = entry.uuid;
            checksum.index_num = i;
            checksum.md5_sum = BlackLibraryCommon::EmptyMD5Version; // TODO check the section index and version number from file name
            checksum.version_num = 0; // TODO read in files and check their current version numbers
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
    else if (command == "export")
    {
        Export(tokens);
    }
    else if (command == "help")
    {
        PrintUsage(tokens);
    }
        else if (command == "import")
    {
        Import(tokens);
    }
    else if (command == "list")
    {
        List(tokens);
    }
    else if (command == "reorder")
    {
        ReorderMd5(tokens);
    }
    else if (command == "size")
    {
        ChangeSize(tokens);
    }
    else if (command == "sizeall")
    {
        ChangeSizeAll(tokens);
    }
    else if (command == "squash")
    {
        SquashMd5Indexes(tokens);
    }
    else if (command == "updateidentifier")
    {
        UpdateMd5Identifier(tokens);
    }
    else if (command == "versionall")
    {
        VersionAll(tokens);
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
