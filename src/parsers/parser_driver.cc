/**
 * parser_driver.cc
 */

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <unordered_map>

#include <Parser.h>

#include <ParserAO3.h>
#include <ParserRR.h>
#include <ParserSBF.h>
#include <ParserSVF.h>
#include <ParserYT.h>

static constexpr const char AO3_SHORT_URL[] = "https://archiveofourown.org/works/505809";
static constexpr const char RR_0_URL[] = "https://www.royalroad.com/fiction/15614/daedalus";
static constexpr const char RR_SHORT_URL[] = "https://www.royalroad.com/fiction/17731/i-never-wanted-you-dead";
static constexpr const char RR_LONG_URL[] = "https://www.royalroad.com/fiction/21220/mother-of-learning";
static constexpr const char SBF_SHORT_URL[] = "https://forums.spacebattles.com/threads/new-operational-parameters.815612/";
static constexpr const char SBF_LONG_URL[] = "https://forums.spacebattles.com/threads/intrepid-worm-au.337516/";
static constexpr const char SVF_LONG_URL[] = "https://forums.sufficientvelocity.com/threads/scientia-weaponizes-the-future.82203/";
static constexpr const char YT_SHORT_URL[] = "https://www.youtube.com/watch?v=GRHat19F2Kg";
static constexpr const char YT_LONG_URL[] = "https://www.youtube.com/playlist?list=PLDb22nlVXGgdg_NR_-GtTrMnbMVmtSSXa";
static constexpr const char EMPTY_URL[] = "";

namespace BlackLibraryParsers = black_library::core::parsers;

using ParserJob = BlackLibraryParsers::ParserJob;

using Parser = BlackLibraryParsers::Parser;
using ParserAO3 = BlackLibraryParsers::AO3::ParserAO3;
using ParserRR = BlackLibraryParsers::RR::ParserRR;
using ParserSBF = BlackLibraryParsers::SBF::ParserSBF;
using ParserSVF = BlackLibraryParsers::SVF::ParserSVF;
using ParserYT = BlackLibraryParsers::YT::ParserYT;

template <typename T>
inline std::shared_ptr<Parser> ParserCast(T const &p)
{
    return std::static_pointer_cast<Parser>(p);
}

struct options
{
    std::string config_path = "";
    BlackLibraryParsers::parser_t source;
    size_t start_number = 1;
    size_t end_number = 0;
    uint8_t length;
    bool print_config = false;
};

static void Usage(const char *prog)
{
    const char *p = strchr(prog, '/');
    printf("usage: %s --(c)onfig --source_(t)arget source --(l)ength 0-2 --section_[s]tart (start number) --section_[e]nd (end number) [-h]\n", p ? (p + 1) : prog);
}

static int ParseOptions(int argc, char **argv, struct options *opts)
{
    static const char *const optstr = "c:e:hl:ps:t:";
    static const struct option long_opts[] = {
        { "config", required_argument, 0, 'c' },
        { "section_end", required_argument, 0, 'e' },
        { "help", no_argument, 0, 'h' },
        { "length", required_argument, 0, 'l' },
        { "print_config", no_argument, 0, 'p' },
        { "section_start", required_argument, 0, 's' },
        { "source_target", required_argument, 0, 't' },
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
            case 'e':
                if (atoi(optarg) < 0)
                {
                    std::cout << "end number out of range" << std::endl;
                    Usage(argv[0]);
                    exit(1);
                }
                opts->end_number = atoi(optarg);
                break;
            case 'h':
                Usage(argv[0]);
                exit(0);
                break;
            case 'l':
                if (atoi(optarg) > 2 || atoi(optarg) < 0)
                {
                    std::cout << "Length out of range" << std::endl;
                    Usage(argv[0]);
                    exit(1);
                }
                opts->length = atoi(optarg);
                break;
            case 'p':
                opts->print_config = true;
                break;
            case 's':
                if (atoi(optarg) < 0)
                {
                    std::cout << "start number out of range" << std::endl;
                    Usage(argv[0]);
                    exit(1);
                }
                opts->start_number = atoi(optarg);
                break;
            case 't':
                if (std::string(optarg) == "ao3")
                {
                    opts->source = BlackLibraryParsers::parser_t::AO3_PARSER;
                }
                if (std::string(optarg) == "empty")
                {
                    opts->source = BlackLibraryParsers::parser_t::ERROR_PARSER;
                }
                else if (std::string(optarg) == "rr")
                {
                    opts->source = BlackLibraryParsers::parser_t::RR_PARSER;
                }
                else if (std::string(optarg) == "sbf")
                {
                    opts->source = BlackLibraryParsers::parser_t::SBF_PARSER;
                }
                else if (std::string(optarg) == "svf")
                {
                    opts->source = BlackLibraryParsers::parser_t::SVF_PARSER;
                }
                else if (std::string(optarg) == "yt")
                {
                    opts->source = BlackLibraryParsers::parser_t::YT_PARSER;
                }
                else
                {
                    std::cout << "Failed to match source" << std::endl;
                    Usage(argv[0]);
                    exit(1);
                }
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

    std::unordered_map<std::string, std::string> url_map;
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::AO3_PARSER) + "1", std::string(AO3_SHORT_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::RR_PARSER) + "0", std::string(RR_0_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::RR_PARSER) + "1", std::string(RR_SHORT_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::RR_PARSER) + "2", std::string(RR_LONG_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::SBF_PARSER) + "1", std::string(SBF_SHORT_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::SBF_PARSER) + "2", std::string(SBF_LONG_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::SVF_PARSER) + "2", std::string(SVF_LONG_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::YT_PARSER) + "1", std::string(YT_SHORT_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::YT_PARSER) + "2", std::string(YT_LONG_URL));
    url_map.emplace(BlackLibraryParsers::GetParserName(BlackLibraryParsers::parser_t::ERROR_PARSER) + "0", std::string(EMPTY_URL));

    std::shared_ptr<Parser> parser;

    if (opts.source == BlackLibraryParsers::parser_t::AO3_PARSER)
    {
        parser = ParserCast(std::make_shared<ParserAO3>(config));
    }
    else if (opts.source == BlackLibraryParsers::parser_t::ERROR_PARSER)
    {
        parser = std::make_shared<Parser>(BlackLibraryParsers::parser_t::ERROR_PARSER, config);
    }
    else if (opts.source == BlackLibraryParsers::parser_t::RR_PARSER)
    {
        parser = ParserCast(std::make_shared<ParserRR>(config));
    }
    else if (opts.source == BlackLibraryParsers::parser_t::SBF_PARSER)
    {
        parser = ParserCast(std::make_shared<ParserSBF>(config));
    }
    else if (opts.source == BlackLibraryParsers::parser_t::SVF_PARSER)
    {
        parser = ParserCast(std::make_shared<ParserSVF>(config));
    }
    else if (opts.source == BlackLibraryParsers::parser_t::YT_PARSER)
    {
        parser = ParserCast(std::make_shared<ParserYT>(config));
    }
    else
    {
        std::cout << "Failed to match parser source" << std::endl;
        Usage(argv[0]);
        exit(1);
    }

    auto iter = url_map.find(BlackLibraryParsers::GetParserName(parser->GetParserType()) + std::to_string(opts.length));

    if (iter == url_map.end())
    {
        std::cout << "Failed to match url" << std::endl;
        std::cout << BlackLibraryParsers::GetParserName(parser->GetParserType()) << std::endl;
        Usage(argv[0]);
        exit(1);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    ParserJob parser_job;
    parser_job.url = iter->second;
    parser_job.last_url = iter->second;
    parser_job.uuid = "some-uuid";
    parser_job.start_number = opts.start_number;
    parser_job.end_number = opts.end_number;

    parser->RegisterProgressNumberCallback(
        [](const std::string &uuid, size_t progress_num, bool error)
        {
            std::cout << "ProgressNumberCallback: " << uuid << " - " << progress_num << " - " << error << std::endl;
        }
    );

    const auto parser_result = parser->Parse(parser_job);

    std::cout << "parser_result: " << parser_result << std::endl;

    curl_global_cleanup();

    std::cout << "Tester exit successful" << std::endl;

    return 0;
}
