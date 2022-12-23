/**
 * db_driver.cc
 * 
 * deprecated 2022-02-14
 */

#include <getopt.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include <ConfigOperations.h>

#include <BlackLibraryDB.h>

namespace BlackLibraryDB = black_library::core::db;

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

    std::ifstream in_file(opts.config_path);
    njson config;
    in_file >> config;

    if (opts.print_config)
    {
        std::cout << config.dump(4) << std::endl;
    }

    BlackLibraryDB::DBEntry create_staging_entry;
    BlackLibraryDB::DBEntry read_staging_entry;
    BlackLibraryDB::DBEntry update_staging_entry;

    std::cout << "Starting db" << std::endl;
    BlackLibraryDB::BlackLibraryDB blacklibrarydb(config);

    create_staging_entry.uuid = "55ee59ad-2feb-4196-960b-3226c65c80d5";
    create_staging_entry.title = "foo2";
    create_staging_entry.nickname = "foo3";
    create_staging_entry.source = "foo4";
    create_staging_entry.url = "foo5";
    create_staging_entry.last_url = "foo8";
    create_staging_entry.series = "the foo";
    create_staging_entry.series_length = 2;
    create_staging_entry.version = 1;
    create_staging_entry.media_path = "foo6";
    create_staging_entry.birth_date = 0;
    create_staging_entry.check_date = 1;
    create_staging_entry.update_date = 2;
    create_staging_entry.user_contributed = 4004;

    // blacklibrarydb.CreateStagingEntry(create_staging_entry);

    // second time should fail
    // blacklibrarydb.CreateStagingEntry(create_staging_entry);

    read_staging_entry = blacklibrarydb.ReadStagingEntry("55ee59ad-2feb-4196-960b-3226c65c80d5");

    std::cout << read_staging_entry << std::endl;

    bool check0 = blacklibrarydb.DoesStagingEntryUrlExist("foo5");
    bool check1 = blacklibrarydb.DoesStagingEntryUrlExist("foo6");

    std::cout << "url foo5 exists: " << check0 << std::endl;
    std::cout << "url foo6 exists: " << check1 << std::endl;

    // update read_staging_entry
    read_staging_entry.title = "2foo";
    read_staging_entry.nickname = "3foo";
    read_staging_entry.source = "4foo";
    read_staging_entry.url = "5foo";
    read_staging_entry.last_url = "8foo";
    read_staging_entry.series = "the bar";
    read_staging_entry.series_length = 3;
    read_staging_entry.version = 2;
    read_staging_entry.media_path = "6foo";
    read_staging_entry.birth_date = 10;
    read_staging_entry.check_date = 11;
    read_staging_entry.update_date = 12;
    read_staging_entry.user_contributed = 4004;

    blacklibrarydb.UpdateStagingEntry(read_staging_entry);

    update_staging_entry = blacklibrarydb.ReadStagingEntry("55ee59ad-2feb-4196-960b-3226c65c80d5");

    std::cout << read_staging_entry << std::endl;

    blacklibrarydb.DeleteStagingEntry("55ee59ad-2feb-4196-960b-3226c65c80d5");
    blacklibrarydb.DeleteStagingEntry("75ee5fad-2deb-4436-120c-3226ceeeaed6");

    read_staging_entry = blacklibrarydb.ReadStagingEntry("55ee59ad-2feb-4196-960b-3226c65c80d5");

    std::cout << read_staging_entry << std::endl;

    std::cout << "Closing application" << std::endl;

    return 0;
}
