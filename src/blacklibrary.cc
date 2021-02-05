/**
 * blacklibrary.cc
 */

#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <string.h>

#include <BlackLibrary.hh>

struct options
{
    std::string db_path = "";
    bool intialize_db = false;
};

static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --(p)ath_db --[i]nit_db [-h]\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "hip:";
    static const struct option long_opts[] = {
        { "help", no_argument, 0, 'h' },
        { "init_db", no_argument, 0, 'i' },
        { "path", required_argument, 0, 'p' },
        { "verbose", no_argument, 0, 'v' },
        { 0, 0, 0, 0 }
    };

    if (!argv || !opts)
        return -1;

    int opt = 0;
    while ((opt = getopt_long(argc, argv, optstr, long_opts, 0)) >= 0)
    {
        switch (opt)
        {
            case 'h':
                Usage(argv[0]);
                exit(0);
                break;
            case 'i':
                opts->intialize_db = true;
                break;
            case 'p':
                opts->db_path = std::string(optarg);
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

int main(int argc, char* argv[])
{
    struct options opts;

    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

    if (ParseOptions(argc, argv, &opts))
    {
        Usage(argv[0]);
        exit(1);
    }

    black_library::BlackLibrary library(opts.db_path, opts.intialize_db);

    blacklibrary = &library;

    blacklibrary->Run();

    return 0;
}