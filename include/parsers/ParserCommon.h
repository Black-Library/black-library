/**
 * ParserCommon.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_COMMON_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_COMMON_H__

#include <time.h>

#include <iostream>
#include <functional>
#include <optional>
#include <string>

#include <libxml/tree.h>
#include <spdlog/fmt/ostr.h>

namespace black_library {

namespace core {

namespace parsers {

enum class job_status_t {
    JOB_ERROR,
    JOB_MANAGER_QUEUED,
    JOB_WORKER_QUEUED,
    JOB_WORKING,
    JOB_FINISHED,

    _NUM_JOB_STATUS_TYPES_
};

std::string GetStatusName(job_status_t job_status);

enum class parser_t {
    ERROR_PARSER,
    AO3_PARSER,
    FFN_PARSER,
    RR_PARSER,
    SBF_PARSER,
    SVF_PARSER,
    WP_PARSER,
    XF_PARSER,
    YT_PARSER,

    _NUM_PARSERS_TYPE
};

enum class parser_behavior_t {
    ERROR,
    INDEX_ENTRY,
    LINKED_LIST
};

enum class pattern_seek_t {
    XML_NAME,
    XML_ATTRIBUTE,
    XML_CONTENT
};

std::ostream& operator<<(std::ostream& out, const pattern_seek_t value);

struct ParseSectionInfo {
    size_t length = 0;
    bool has_error = true;
};

struct ParserIndexEntry {
    std::string data_url;
    std::string name;
    time_t time_published = 0;
    size_t index_num;
};

typedef bool error_job_rep;

struct ParserJob {
    std::string uuid;
    std::string url;
    std::string last_url;
    size_t start_number = 1;
    size_t end_number = 0;
    error_job_rep is_error_job = false;
};

struct ParserJobHash
{
    std::size_t operator() (const ParserJob &parser_job) const
    {
        std::size_t h1 = std::hash<std::string>()(parser_job.uuid);
        std::size_t h2 = std::hash<std::string>()(parser_job.url);
        std::size_t h3 = std::hash<std::string>()(parser_job.last_url);
        std::size_t h4 = std::hash<size_t>()(parser_job.start_number);
        std::size_t h5 = std::hash<size_t>()(parser_job.end_number);
        std::size_t h6 = std::hash<error_job_rep>()(parser_job.is_error_job);

        return h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6;
    }
};

struct ParserJobStatusTracker
{
    std::string uuid;
    job_status_t job_status;
    error_job_rep is_error_job = false;
};

inline std::ostream& operator << (std::ostream &o, const ParserJobStatusTracker &parser_job_status_tracker)
{
    o << "uuid: " << parser_job_status_tracker.uuid << " ";
    o << "job_status: " << GetStatusName(parser_job_status_tracker.job_status) << " ";
    o << "is_error_job: " << parser_job_status_tracker.is_error_job;

    return o;
}

struct CurrentJobPairHash
{
    std::size_t operator() (const std::pair<const std::string, error_job_rep> &pair) const
    {
        std::size_t h1 = std::hash<std::string>()(pair.first);
        std::size_t h2 = std::hash<error_job_rep>()(pair.second);

        return h1 ^ h2;
    }
};

inline bool operator == (const ParserJob &left, const ParserJob &right)
{
    return left.uuid == right.uuid && left.url == right.url &&
        left.start_number == right.start_number && left.end_number == right.end_number;
}

inline std::ostream& operator << (std::ostream &o, const ParserJob &parser_job)
{
    o << "uuid: " << parser_job.uuid << " ";
    o << "url: " << parser_job.url << " ";
    o << "last_url: " << parser_job.last_url << " ";
    o << "start_number: " << parser_job.start_number << " ";
    o << "end_number: " << parser_job.end_number << " ";
    o << "is_error_job: " << parser_job.is_error_job;

    return o;
}

struct ParserResultMetadata {
    std::string uuid;
    std::string title;
    std::string author;
    std::string nickname = "";
    std::string source;
    std::string url;
    std::string last_url;
    std::string series;
    uint16_t series_length = 1;
    std::string media_path;
    time_t update_date = 0;
};

inline std::ostream& operator << (std::ostream &o, const ParserResultMetadata &parser_metadata)
{
    o << "uuid: " << parser_metadata.uuid << " ";
    o << "title: " << parser_metadata.title << " ";
    o << "author: " << parser_metadata.author << " ";
    o << "nickname: " << parser_metadata.nickname << " ";
    o << "source: " << parser_metadata.source << " ";
    o << "url: " << parser_metadata.url << " ";
    o << "last_url: " << parser_metadata.last_url << " ";
    o << "series_length: " << parser_metadata.series_length << " ";
    o << "media_path: " << parser_metadata.media_path << " ";
    o << "update_date: " << parser_metadata.update_date;

    return o;
}

struct ParserJobResult {
    ParserResultMetadata metadata;

    size_t start_number = 0;
    size_t end_number = 0;

    error_job_rep is_error_job = false;
    bool has_error = true;
};

inline std::ostream& operator << (std::ostream &o, const ParserJobResult &job_result)
{
    o << "metadata: " << job_result.metadata << " ";
    o << "start_number: " << job_result.start_number << " ";
    o << "end_number: " << job_result.end_number << " ";
    o << "is_error_job: " << job_result.is_error_job << " ";
    o << "has_error: " << job_result.has_error;

    return o;
}

struct ParserResult {
    ParserResultMetadata metadata;

    std::string debug_string;
    error_job_rep is_error_job = false;
    bool has_error = true;
};

inline std::ostream& operator << (std::ostream &o, const ParserResult &parser_result)
{
    o << "metadata: " << parser_result.metadata << " ";
    o << "debug_string: " << parser_result.debug_string << " ";
    o << "is_error_job: " << parser_result.is_error_job << " ";
    o << "has_error: " << parser_result.has_error;

    return o;
}

struct ParserXmlAttributeResult {
    std::string result = "";
    bool found = false;
};

struct ParserXmlContentResult {
    std::string result = "";
    bool found = false;
};

struct ParserXmlNodeSeek {
    xmlNodePtr seek_node = NULL;
    bool found = false;
};

struct ParserTimeResult {
    time_t time = {0};
    bool found = false;
};

// TODO check using vs typedef
using database_status_callback = std::function<void(ParserJobResult result)>;
using job_status_callback = std::function<void(const ParserJob &parser_job, job_status_t job_status)>;
using manager_notify_callback = std::function<void(ParserJobResult result)>;
using progress_number_callback = std::function<void(const std::string &uuid, size_t progress_num, bool error)>;
using version_read_callback = std::function<std::string(const std::string &uuid, size_t index_num)>;
using version_read_num_callback = std::function<uint16_t(const std::string &uuid, size_t index_num)>;
using version_update_callback = std::function<void(const std::string &uuid, size_t index_num, const std::string &md5_sum, uint64_t version_num)>;

std::string GenerateXmlDocTreeString(xmlNodePtr root_node);
std::string GenerateXmlDocTreeStringHelper(xmlNodePtr root_node, size_t depth);

std::string GetParserBehaviorName(parser_behavior_t behavior);
std::string GetParserName(parser_t rep);
parser_t GetParserTypeByUrl(const std::string &url);
std::string GetSectionFileName(const size_t &index_num, const std::string &section_name, const uint16_t &version_num);
std::string GetSpaceString(size_t num_tabs);
ParserXmlNodeSeek SeekToNodeByElementAttr(xmlNodePtr root, std::string attr, std::string value);
ParserXmlContentResult GetXmlNodeContent(xmlNodePtr root_node);
ParserXmlAttributeResult GetXmlAttributeContentByName(xmlNodePtr root_node, const std::string &target_name);

bool NodeHasAttribute(xmlNodePtr root_node, const std::string &target_name);
bool NodeHasAttributeContent(xmlNodePtr root_node, const std::string &target_content);

ParserXmlNodeSeek SeekToNodeByName(xmlNodePtr root_node, const std::string &name);
ParserXmlNodeSeek SeekToNodeByNameRecursive(xmlNodePtr root_node, const std::string &name);

template<typename ... Args>
ParserXmlNodeSeek SeekToNodeByPattern(xmlNodePtr root_node, Args... args)
{
    ParserXmlNodeSeek result;
    if (!root_node)
    {
        return result;
    }

    xmlNodePtr current_node = NULL;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        const auto children_seek = SeekToNodeByPattern(current_node->children, args...);
        if (children_seek.found)
        {
            result.seek_node = children_seek.seek_node;
            result.found = true;
            return result;
        }

        bool found = SeekToNodeByPatternHelper(current_node, args...);
        if (found)
        {
            result.seek_node = current_node;
            result.found = true;
            return result;
        }
    }

    return result;
}

bool SeekToNodeByPatternHelper(xmlNodePtr root_node);
bool SeekToNodeByPatternHelper(xmlNodePtr root_node, std::string match);

template<typename ... Args>
bool SeekToNodeByPatternHelper(xmlNodePtr root_node, pattern_seek_t pattern, std::string match, Args... args)
{
    switch (pattern)
    {
        case pattern_seek_t::XML_NAME:
        {
            if (xmlStrcmp(root_node->name, (const xmlChar *) match.c_str()))
            {
                return false;
            }
            break;
        }
        case pattern_seek_t::XML_CONTENT:
        {
            ParserXmlContentResult content_result = GetXmlNodeContent(root_node);
            if (xmlStrcmp((const xmlChar *) content_result.result.c_str(), (const xmlChar *) match.c_str()))
            {
                return false;
            }
            break;
        }
        case pattern_seek_t::XML_ATTRIBUTE:
        {
            // Split the string
            std::string attribute = match;
            std::string attr = attribute.substr(0, attribute.find('='));
            std::string value = attribute.substr(attribute.find('=') + 1);

            bool found = false;
            xmlAttrPtr prop = root_node->properties;
            while (prop)
            {
                // std::cout << "name: " << prop->name << " attr: " << attr << " content: " << prop->children->content << " value: " << value <<std::endl;
                if (!xmlStrcmp(prop->name, (const xmlChar *) attr.c_str()) &&
                    !(xmlStrcmp(prop->children->content, (const xmlChar *) value.c_str())))
                {
                    found = true;
                    break;
                }

                prop = prop->next;
            }
            if (!found)
            {
                return false;
            }
            break;
        }
        default:
            return false;
    }

    return SeekToNodeByPatternHelper(root_node, args...);
}

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
