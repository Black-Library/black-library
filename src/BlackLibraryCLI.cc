/**
 * BlackLibraryCLI.cc
 */

#include <iostream>

#include <BlackLibraryCLI.h>

namespace black_library {

BlackLibraryCLI::BlackLibraryCLI(const std::string &db_path, const std::string &storage_path) :
    blacklibrary_db_(db_path, false),
    storage_path_(storage_path),
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

        ProcessInput(tokens);
    }

    return 0;
}

int BlackLibraryCLI::Stop()
{
    done_ = true;

    return 0;
}

void BlackLibraryCLI::PrintEntries(const std::vector<std::string> &tokens)
{
    std::string target_entry_type = tokens[1];
    std::string entry_list;

    if (target_entry_type == "black")
        entry_list = blacklibrary_db_.GetBlackEntryList();
    else if (target_entry_type == "staging")
        entry_list = blacklibrary_db_.GetStagingEntryList();
    
    std::cout << entry_list << std::endl;
}

void BlackLibraryCLI::PrintUsage(const std::vector<std::string> &tokens)
{
    std::cout << "Usage: " << std::endl; 

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << tokens[i] << ' ';
    }

    std::cout << std::endl;
}

void BlackLibraryCLI::ProcessInput(const std::vector<std::string> &tokens)
{
    std::string command = tokens[0];
    if (command == "stop")
    {
        Stop();
    }
    if (command == "list")
    {
        PrintEntries(tokens);
    }
    else
    {
        PrintUsage(tokens);
    }
    std::cout << "COMMAND: " << command << std::endl;
}

} // namespace black_library
