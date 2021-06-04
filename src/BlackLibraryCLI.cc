/**
 * BlackLibraryCLI.cc
 */

#include <algorithm>
#include <iostream>
#include <sstream>

#include <FileOperations.h>

#include <BlackLibraryCLI.h>

namespace black_library {

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

void BlackLibraryCLI::PrintEntries(const std::vector<std::string> &tokens)
{
    std::string entry_list;
    std::string target_entry_type;
    if (tokens.size() >= 2)
    {
        target_entry_type = tokens[1];
    }

    if (target_entry_type == "black")
        entry_list = blacklibrary_db_.GetBlackEntryList();
    else if (target_entry_type == "staging")
        entry_list = blacklibrary_db_.GetStagingEntryList();
    else if (target_entry_type == "help")
        entry_list = "print [black, staging]";
    else
    {
        entry_list += blacklibrary_db_.GetStagingEntryList();
        entry_list += entry_list = blacklibrary_db_.GetBlackEntryList();
    }
    
    std::cout << entry_list << std::endl;
}

void BlackLibraryCLI::PrintUsage(const std::vector<std::string> &tokens)
{
    std::stringstream ss;

    ss << "Usage: [bind, help, list]";

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
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\0'), token.end());
        token.erase(std::remove(token.begin(), token.end(), ';'), token.end());

        // remove unprintable characters
        token.erase(std::remove_if(token.begin(), token.end(), 
            [](unsigned char c)
            {
                return !std::isprint(c);
            })
            , token.end());
    }
}

} // namespace black_library
