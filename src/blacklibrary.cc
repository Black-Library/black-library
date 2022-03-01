/**
 * blacklibrary.cc
 */

#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <string.h>

#include <BlackLibrary.h>

struct options
{
    std::string config_path = "";
    bool print_config = false;
};

static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --(c)onfig --[p]rint_config -[h]elp\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "c:ph";
    static const struct option long_opts[] = {
        { "config", required_argument, 0, 'c' },
        { "help", no_argument, 0, 'h' },
        { "print_config", no_argument, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    if (!argv || !opts)
        return -1;

    int opt = 0;
    while ((opt = getopt_long(argc, argv, optstr, long_opts, 0)) >= 0)
    {
        switch (opt)
        {
            case 'c':
                opts->config_path = std::string(optarg);
                break;
            case 'h':
                Usage(argv[0]);
                exit(0);
                break;
            case 'p':
                opts->print_config = true;
                break;
            default:
                exit(1);
                break;
        }
    }

    if (optind < argc)
    {
        fprintf(stderr, "trailing options..\n");
        exit(1);
    }

    return 0;
}

black_library::BlackLibrary *blacklibrary;

void SigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT)
        blacklibrary->Stop();
}

int main(int argc, char *argv[])
{
    struct options opts;

    if (argc < 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    if (ParseOptions(argc, argv, &opts))
    {
        Usage(argv[0]);
        exit(1);
    }

    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

    std::ifstream in_file(opts.config_path);
    njson rconfig;
    in_file >> rconfig;

    if (opts.print_config)
    {
        std::cout << rconfig.dump(4) << std::endl;
    }

    black_library::BlackLibrary library(rconfig);

    blacklibrary = &library;

    blacklibrary->Run();

    return 0;
}