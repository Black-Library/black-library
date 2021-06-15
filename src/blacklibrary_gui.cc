/**
 * blacklibrary_gui.cc
 */

#include <getopt.h>
#include <string.h>

#include <BlackLibraryGUI.h>

struct options
{
    std::string db_path = "";
    std::string storage_path = "";
};

black_library::BlackLibraryGUI *blacklibrary_gui;


static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --path_(d)b --path_(s)torage [-h]\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "d:hs:";
    static const struct option long_opts[] = {
        { "path_db", required_argument, 0, 'd' },
        { "help", no_argument, 0, 'h' },
        { "path_storage", required_argument, 0, 's' },
        { 0, 0, 0, 0 }
    };

    if (!argv || !opts)
        return -1;

    int opt = 0;
    while ((opt = getopt_long(argc, argv, optstr, long_opts, 0)) >= 0)
    {
        switch (opt)
        {
            case 'd':
                opts->db_path = std::string(optarg);
                break;
            case 'h':
                Usage(argv[0]);
                exit(0);
                break;
            case 's':
                opts->storage_path = std::string(optarg);
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

int main(int argc, char* argv[])
{
    struct options opts;

    if (ParseOptions(argc, argv, &opts))
    {
        Usage(argv[0]);
        exit(1);
    }

    black_library::BlackLibraryGUI gui(opts.db_path, opts.storage_path);

    blacklibrary_gui = &gui;

    blacklibrary_gui->Run();

    return 0;
}