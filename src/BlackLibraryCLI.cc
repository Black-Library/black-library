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
    std::string input;

    while (!done_)
    {
        std::cout << "BlackLibraryCLI>";

        std::getline(std::cin, input);

        ProcessInput(input);
    }

    return 0;
}

int BlackLibraryCLI::Stop()
{
    done_ = true;

    return 0;
}

void BlackLibraryCLI::ProcessInput(const std::string &input)
{
    if (input == "stop")
    {
        Stop();
    }
    std::cout << "ECHO: " << input << std::endl;
}

} // namespace black_library
