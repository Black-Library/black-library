/**
 * parser_manager_driver.cc
 */

#include <getopt.h>

#include <iostream>
#include <signal.h>

#include <ConfigOperations.h>

#include <ParserManager.h>

namespace BlackLibraryParsers = black_library::core::parsers;

BlackLibraryParsers::ParserManager *parser_manager;

struct options
{
    std::string config_path = "";
    bool print_config = false;
};

static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --(c)onfig --[p]rint_config [-h]\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "c:hp";
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

void SigHandler(int sig)
{
    if (sig == SIGTERM || sig == SIGINT)
        parser_manager->Stop();
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
    njson config;
    in_file >> config;

    if (opts.print_config)
    {
        std::cout << config.dump(4) << std::endl;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    BlackLibraryParsers::ParserManager manager(config);

    parser_manager = &manager;

    parser_manager->RegisterProgressNumberCallback(
        [](const std::string &uuid, size_t progress_num, bool error)
        {
            std::cout << "ProgressNumberCallback: " << uuid << " - " << progress_num << " - " << error << std::endl;
        }
    );
    parser_manager->RegisterDatabaseStatusCallback(
        [](BlackLibraryParsers::ParserJobResult result)
        {
            std::cout << "DatabaseStatusCallback: " << result.metadata.uuid << std::endl;
        }
    );

    parser_manager->AddJob("some-uuid-0", "https://www.fanfiction.net/s/7347955/1/Dreaming-of-Sunshine", "https://www.fanfiction.net/s/7347955/1/Dreaming-of-Sunshine");

    parser_manager->AddJob("some-uuid-11", "https://www.royalroad.com/fiction/21220/mother-of-learning", "https://www.royalroad.com/fiction/21220/mother-of-learning");
    parser_manager->AddJob("some-uuid-12", "https://www.royalroad.com/fiction/42266/elysium", "https://www.royalroad.com/fiction/42266/elysium");
    parser_manager->AddJob("some-uuid-13", "https://www.royalroad.com/fiction/17731/i-never-wanted-you-dead", "https://www.royalroad.com/fiction/17731/i-never-wanted-you-dead");
    // parser_manager->AddJob("fcda9e40-0307-452-bd9b-f77c3a4c4492", "https://www.royalroad.com/fiction/17731/i-never-wanted-you-dead", "https://www.royalroad.com/fiction/17731/i-never-wanted-you-dead");
    parser_manager->AddJob("some-uuid-13", "https://www.royalroad.com/fiction/30692/divinity", "https://www.royalroad.com/fiction/30692/divinity");
    parser_manager->AddJob("some-uuid-14", "https://www.royalroad.com/fiction/14167/metaworld-chronicles", "https://www.royalroad.com/fiction/14167/metaworld-chronicles");

    parser_manager->AddJob("some-uuid-20", "https://forums.spacebattles.com/threads/commander-pa-multicross-si.309838/", "https://forums.spacebattles.com/threads/commander-pa-multicross-si.309838/");
    parser_manager->AddJob("some-uuid-21", "https://forums.spacebattles.com/threads/be-thou-my-good.867883/", "https://forums.spacebattles.com/threads/be-thou-my-good.867883/");
    parser_manager->AddJob("some-uuid-22", "https://forums.spacebattles.com/threads/mha-worlds-finest-bnhaxbatman-complete.868273/", "https://forums.spacebattles.com/threads/mha-worlds-finest-bnhaxbatman-complete.868273/");
    parser_manager->AddJob("some-uuid-23", "https://forums.spacebattles.com/threads/perchance-to-dream-mass-effect-commander.664360/", "https://forums.spacebattles.com/threads/perchance-to-dream-mass-effect-commander.664360/");

    parser_manager->AddJob("some-uuid-30", "https://forums.sufficientvelocity.com/threads/scientia-weaponizes-the-future.82203/", "https://forums.sufficientvelocity.com/threads/scientia-weaponizes-the-future.82203/");

    parser_manager->Run();

    curl_global_cleanup();

    std::cout << "parser_manager_driver exit successful" << std::endl;

    return 0;
}
