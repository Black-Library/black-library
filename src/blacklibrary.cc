/**
 * blacklibrary.cc
 */

#include <getopt.h>
#include <iostream>
#include <signal.h>

#include <BlackLibrary.hh>

black_library::BlackLibrary *blacklibrary;


void SigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT)
        blacklibrary->Stop();
}

int main(int argc, char* argv[])
{
    // check to see if there are any extra arguments
    if (argc > 1)
    {
        std::cout << "extra arguments ";
        for (int i = 0; i < argc; ++i)
        {
            std::cout << std::string(argv[i]);
        }
        std::cout << std::endl;
    }

    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

    blacklibrary->Run();

    return 0;
}