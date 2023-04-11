/**
 * Parser.cc
 */

#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include <LogOperations.h>
#include <VersionOperations.h>

#include <Parser.h>
#include <ShortTimeGenerator.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

// TODO user parser metadata result instead of individual fields
// TODO restructure main parser loop -> parse section, version check section, save section

Parser::Parser(parser_t parser_type, const njson &config) :
    md5s_(),
    md5_check_callback_(),
    md5_read_callback_(),
    md5s_read_callback_(),
    md5_update_callback_(),
    progress_number_callback_(),
    time_generator_(std::make_shared<ShortTimeGenerator>()),
    uuid_(""),
    title_(GetParserName(parser_type) + "_title"),
    nickname_(""),
    source_name_(BlackLibraryCommon::ERROR::source_name),
    source_url_(BlackLibraryCommon::ERROR::source_url),
    author_("unknown-author"),
    target_url_(""),
    local_des_(""),
    parser_name_(""),
    last_update_date_(0),
    index_(0),
    target_start_index_(0),
    target_end_index_(0),
    parser_type_(parser_type),
    parser_behavior_(parser_behavior_t::ERROR),
    done_(false),
    first_curl_wait_done_(false)
{
    njson nconfig = BlackLibraryCommon::LoadConfig(config);

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    bool logger_level = BlackLibraryCommon::DefaultLogLevel;
    if (nconfig.contains("parser_debug_log"))
    {
        logger_level = nconfig["parser_debug_log"];
    }

    parser_name_ = GetParserName(parser_type_);

    BlackLibraryCommon::InitRotatingLogger(parser_name_, logger_path, logger_level);
}

Parser::Parser(const Parser &parser) :
    parser_type_(parser.parser_type_),
    parser_behavior_(parser.parser_behavior_),
    done_(bool(parser.done_))
{
}

Parser::~Parser()
{
    done_ = true;
}

ParserResult Parser::Parse(const ParserJob &parser_job)
{
    const std::lock_guard<std::mutex> lock(mutex_);
    ParserResult parser_result;
    done_ = false;
    first_curl_wait_done_ = false;

    uuid_ = parser_job.uuid;

    parser_result.metadata.url = parser_job.url;
    parser_result.metadata.uuid = parser_job.uuid;
    parser_result.metadata.media_path = local_des_;
    parser_result.is_error_job = parser_job.is_error_job;

    target_url_ = PreprocessTargetUrl(parser_job);

    if (CalculateIndexBounds(parser_job))
    {
        return parser_result;
    }

    BlackLibraryCommon::LogDebug(parser_name_, "Start parser job: {} target_url: {}", parser_job, target_url_);

    const auto curl_result = CurlRequest(target_url_);

    xmlDocPtr doc_tree = htmlReadDoc((xmlChar*) curl_result.c_str(), NULL, NULL,
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc_tree == NULL)
    {
        BlackLibraryCommon::LogError(parser_name_, "libxml HTMLparser unable to parse url: {}", parser_job.url);
        return parser_result;
    }

    // const xmlChar* encoding = doc_tree->encoding;

    xmlNodePtr root_node = xmlDocGetRootElement(doc_tree);
    xmlNodePtr current_node = root_node->children;

    BlackLibraryCommon::LogDebug(parser_name_, "Find metadata from url: {}", target_url_);

    FindMetaData(current_node);

    // reset current node ptr to root node children
    current_node = root_node->children;

    if (PreParseLoop(current_node, parser_job))
    {
        BlackLibraryCommon::LogError(parser_name_, "PreParseLoop failure");
        return parser_result;
    }

    // reset current node ptr to root node children
    current_node = root_node->children;

    SaveMetaData(parser_result);

    xmlFreeDoc(doc_tree);

    // wait before parsing any sections
    FirstCurlWait();

    ParseLoop(parser_result);

    PostParseLoop(parser_result);

    // save information to parser_result
    SaveLastUrl(parser_result);
    SaveUpdateDate(parser_result);

    parser_result.has_error = false;

    return parser_result;
}

void Parser::Stop()
{
    done_ = true;
    first_curl_wait_done_ = true;
}

std::string Parser::CurlRequest(const std::string &url)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (!curl)
    {
        BlackLibraryCommon::LogError(parser_name_, "Curl request failed, curl did not intialize");
        return "";
    }

    std::string html_raw;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HandleCurlResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_raw);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {

        BlackLibraryCommon::LogError(parser_name_, "Curl request failed: {}", curl_easy_strerror(res));
        return "";
    }

    curl_easy_cleanup(curl);

    return html_raw;
}

void Parser::SetLocalFilePath(const std::string &local_des)
{
    const std::lock_guard<std::mutex> lock(mutex_);
    local_des_ = local_des;
}

bool Parser::GetDone()
{
    return done_;
}

parser_behavior_t Parser::GetParserBehaviorType()
{
    return parser_behavior_;
}

parser_t Parser::GetParserType()
{
    return parser_type_;
}

std::string Parser::GetSourceName()
{
    return source_name_;
}

std::string Parser::GetSourceUrl()
{
    return source_url_;
}

int Parser::RegisterMd5CheckCallback(const md5_check_callback &callback)
{
    md5_check_callback_ = callback;

    return 0;
}

int Parser::RegisterMd5ReadCallback(const md5_read_callback &callback)
{
    md5_read_callback_ = callback;

    return 0;
}

int Parser::RegisterMd5sReadCallback(const md5s_read_callback &callback)
{
    md5s_read_callback_ = callback;

    return 0;
}

int Parser::RegisterMd5UpdateCallback(const md5_update_callback &callback)
{
    md5_update_callback_ = callback;

    return 0;
}

int Parser::RegisterProgressNumberCallback(const progress_number_callback &callback)
{
    progress_number_callback_ = callback;

    return 0;
}

int Parser::RegisterVersionReadNumCallback(const version_read_num_callback &callback)
{
    version_read_num_callback_ = callback;

    return 0;
}

std::string Parser::SectionDumpContent(const xmlDocPtr doc_ptr, const xmlNodePtr node_ptr)
{
    xmlBufferPtr section_buf = xmlBufferCreate();
    xmlNodeDump(section_buf, doc_ptr, node_ptr, 0, 1);
    const std::string section_content = std::string((char *) section_buf->content);
    xmlBufferFree(section_buf);

    return section_content;
}

bool Parser::SectionFileSave(const std::string &section_content, const std::string &section_file_name)
{
    const std::string file_path = local_des_ + section_file_name;
    BlackLibraryCommon::LogDebug(parser_name_, "FILEPATH: {}", file_path);

    std::ofstream ofs;
    ofs.open(file_path);
    if (!ofs.is_open())
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to open file with path: {}", file_path);
        return true;
    }
    ofs << section_content;
    ofs.close();

    return false;
}

int Parser::CalculateIndexBounds(const ParserJob &parser_job)
{
    index_ = parser_job.start_number - 1;

    return 0;
}

void Parser::ExpendedAttempts()
{
    done_ = true;
}

void Parser::FindMetaData(xmlNodePtr root_node)
{
    (void) root_node;
}

void Parser::IndicateNextSection()
{
    return;
}

ParseSectionInfo Parser::ParseSection()
{
    ParseSectionInfo parse_section_info;
    return parse_section_info;
}

void Parser::ParseLoop(ParserResult &parser_result)
{
    size_t seconds_counter = 0;
    size_t wait_time = 0;
    size_t wait_time_total = 0;
    size_t remaining_attempts = 5;

    while (!done_)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);

        if (done_)
            break;

        if (seconds_counter >= wait_time)
            seconds_counter = 0;

        if (seconds_counter == 0)
        {
            // let the fake reader finish waiting before exiting
            if (ReachedEnd())
            {
                done_ = true;
                BlackLibraryCommon::LogDebug(parser_name_, "UUID: {} reached end", uuid_);
                continue;
            }

            // exit if no remaining attempts
            if (remaining_attempts <= 0)
            {
                BlackLibraryCommon::LogError(parser_name_, "Max failures for UUID: {} index: {} reached", uuid_, index_);
                remaining_attempts = 5;
                ExpendedAttempts();
                continue;
            }

            ParseSectionInfo parse_section_info = ParseSection();
            --remaining_attempts;

            wait_time = time_generator_->GenerateWaitTime(parse_section_info.length);
            wait_time_total += wait_time;

            if (parse_section_info.has_error)
            {
                BlackLibraryCommon::LogError(parser_name_, "Failed to parse section for UUID: {} index: {} remaining attempts: {} waiting {} seconds - wait time total: {} seconds", 
                    uuid_, index_, remaining_attempts, wait_time, wait_time_total);

                if (remaining_attempts == 0 && progress_number_callback_)
                    progress_number_callback_(uuid_, index_ + 1, true);
            }
            else
            {
                BlackLibraryCommon::LogDebug(parser_name_, "Title: {} index: {} section length: {} waiting {} seconds - wait time total: {} seconds",
                    title_, index_, parse_section_info.length, wait_time, wait_time_total);

                if (progress_number_callback_)
                    progress_number_callback_(uuid_, index_ + 1, false);

                parser_result.metadata.series_length = index_ + 1;

                remaining_attempts = 5;
                IndicateNextSection();
            }
        }

        ++seconds_counter;

        std::this_thread::sleep_until(deadline);
    }
}

void Parser::PostParseLoop(ParserResult &parser_result)
{
    (void) parser_result;
    BlackLibraryCommon::LogDebug(parser_name_, "Post parse loop");
    return;
}

int Parser::PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job)
{
    (void) root_node;
    (void) parser_job;
    return 1;
}

std::string Parser::PreprocessTargetUrl(const ParserJob &parser_job)
{
    return parser_job.url;
}

bool Parser::ReachedEnd()
{
    return true;
}

void Parser::SaveLastUrl(ParserResult &parser_result)
{
    (void) parser_result;
    return;
}

void Parser::SaveMetaData(ParserResult &parser_result)
{
    BlackLibraryCommon::LogDebug(parser_name_, "Title: {} Author: {} Nickname: {}", title_, author_, nickname_);

    parser_result.metadata.title = title_;
    parser_result.metadata.author = author_;
    parser_result.metadata.nickname = nickname_;
    parser_result.metadata.source = GetSourceName();
}

void Parser::SaveUpdateDate(ParserResult &parser_result)
{
    (void) parser_result;
    return;
}

int Parser::FirstCurlWait()
{
    size_t seconds_counter = 0;
    size_t wait_time = 15;

    while (!first_curl_wait_done_)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);

        if (first_curl_wait_done_)
            break;

        if (seconds_counter >= wait_time)
        {
            first_curl_wait_done_ = true;
        }

        ++seconds_counter;

        std::this_thread::sleep_until(deadline);
    }

    return 0;
}

// Credit: https://stackoverflow.com/questions/5525613/how-do-i-fetch-a-html-page-source-with-libcurl-in-c
size_t HandleCurlResponse(void* ptr, size_t size, size_t nmemb, void* data)
{
    std::string* str = (std::string*) data;
    char* sptr = (char*) ptr;

    for (size_t x = 0; x < size * nmemb; ++x)
    {
        (*str) += sptr[x];
    }

    return size * nmemb;
}

} // namespace parsers
} // namespace core
} // namespace black_library
