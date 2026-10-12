/**
 * binder_driver.cc
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include <BlackLibraryBinder.h>

namespace BlackLibraryBinder = black_library::core::binder;

struct options
{
    std::string config_path = "";
    std::string uuid;
};

static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --(c)onfig --(u)uid uuid [-h]\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "c:hu:";
    static const struct option long_opts[] = {
        { "config", required_argument, 0, 'c'},
        { "help", no_argument, 0, 'h' },
        { "uuid", required_argument, 0, 'u' },
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
            case 'u':
                opts->uuid = std::string(optarg);
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

int main(int argc, char *argv[])
{
    struct options opts;

    if (argc > 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    if (ParseOptions(argc, argv, &opts))
    {
        Usage(argv[0]);
        exit(1);
    }

    // std::ifstream in_file(opts.config_path);
    njson rconfig;
    // in_file >> rconfig;

    BlackLibraryBinder::BlackLibraryBinder binder(rconfig);

    binder.Bind(opts.uuid, "test-0");

    return 0;
}