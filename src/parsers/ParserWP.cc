/**
 * ParserWP.cc
 */

#include <chrono>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>

#include <FileOperations.h>
#include <LogOperations.h>
#include <VersionOperations.h>

#include <ParserWP.h>

namespace black_library {

namespace core {

namespace parsers {

namespace WP {

namespace BlackLibraryCommon = black_library::core::common;

ParserWP::ParserWP(const njson &config) :
    LinkedListParser(parser_t::WP_PARSER, config)
{
    source_name_ = BlackLibraryCommon::WP::source_name;
    source_url_ = BlackLibraryCommon::WP::source_url;
}

void ParserWP::FindMetaData(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {

        BlackLibraryCommon::LogWarn(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return;
    }
    current_node = body_seek.seek_node;

    const auto span_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "span",
        pattern_seek_t::XML_ATTRIBUTE, "class=author vcard");
    if (!span_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed author span seek for UUID: {}", uuid_);
        return;
    }
    current_node = span_seek.seek_node;

    const auto author_seek = SeekToNodeByName(current_node, "a");
    if (!author_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed author seek for UUID: {}", uuid_);
        return;
    }
    current_node = author_seek.seek_node;

    const auto author_result = GetXmlNodeContent(current_node);
    if (!author_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get author content for UUID: {}", uuid_);
        return;
    }

    author_ = author_result.result;
}

ParseSectionInfo ParserWP::ParseSection()
{
    ParseSectionInfo output;

    const auto working_url = next_url_;
    const auto working_index = index_;

    BlackLibraryCommon::LogDebug(parser_name_, "ParseSection: {} section_url: {}", GetParserBehaviorName(parser_behavior_), working_url);

    if (working_index > target_end_index_)
    {
        reached_end_ = true;
        output.has_error = false;
        return output;
    }

    // Get html
    const auto curl_request_result = CurlRequest(working_url);
    xmlDocPtr section_doc_tree = htmlReadDoc((xmlChar*) curl_request_result.c_str(), NULL, NULL,
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (section_doc_tree == NULL)
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to parse into doc_tree for UUID: {}", uuid_);
        return output;
    }

    xmlNodePtr root_node = xmlDocGetRootElement(section_doc_tree);
    xmlNodePtr current_node = root_node->children;
    xmlNodePtr length_node = NULL;
    size_t length = 0;

    // Get main Section
    const auto p_body_main_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "main",
        pattern_seek_t::XML_ATTRIBUTE, "id=main");
    if (!p_body_main_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed p-body-main seek for UUID: {}", uuid_);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    current_node = p_body_main_seek.seek_node;

    // get title
    const auto section_title = GetSectionTitle(current_node->children);
    if (section_title.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get section title");
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    // get next url
    next_url_ = GetNextUrl(current_node->children);

    // get last update date
    last_update_date_ = GetUpdateDate(current_node->children);

    // skip saving content if before target start index
    BlackLibraryCommon::LogDebug(parser_name_, "working index: {} start index: {}", working_index, target_start_index_);
    if (working_index <= target_start_index_)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "Target start index: {} - current index: {} skipping file save", target_start_index_, working_index);
        output.has_error = false;
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    // Get story content node
    const auto section_content_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=entry-content");
    if (!section_content_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed section content seek for UUID: {}", uuid_);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    current_node = section_content_seek.seek_node;

    for (length_node = current_node->children; length_node; length_node = length_node->next)
    {
        if (length_node->type != XML_ELEMENT_NODE)
            continue;
        ++length;
    }

    output.length = length;

    const auto sanatized_section_name = BlackLibraryCommon::SanitizeFileName(section_title);

    if (sanatized_section_name.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to generate section name from: {}", section_title);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    std::string saved_version = BlackLibraryCommon::EmptyMD5Version;
    bool skip_file_check = false;

    if (version_read_callback_)
        saved_version = version_read_callback_(uuid_, index_);

    if (saved_version == BlackLibraryCommon::EmptyMD5Version)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "No MD5 sum for: {} index: {}", uuid_, index_);
    }

    // dump content
    auto section_content = SectionDumpContent(section_doc_tree, current_node);
    xmlFreeDoc(section_doc_tree);

    if (section_content.empty())
    {
        return output;
    }

    // version check
    auto sec_version = BlackLibraryCommon::GetMD5Hash(section_content);
    BlackLibraryCommon::LogDebug(parser_name_, "Section UUID: {} index: {} checksum hash: {}", uuid_, index_, sec_version);

    if (saved_version == sec_version)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "Version hash matches: {} index: {}, skip file save", uuid_, index_);
        skip_file_check = true;
    }

    // if we skip the file check we can just return after freeing the doc
    if (skip_file_check)
    {
        output.has_error = false;
        return output;
    }

    uint16_t version_num = 0;

    if (version_read_num_callback_)
        version_num = version_read_num_callback_(uuid_, index_);

    const auto section_file_name = GetSectionFileName(working_index, sanatized_section_name, 0);

    if (SectionFileSave(section_content, section_file_name))
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed section file save with UUID: {}", uuid_);
        return output;
    }

    if (version_update_callback_)
        version_update_callback_(uuid_, index_, sec_version, version_num);

    output.has_error = false;

    return output;
}

std::string ParserWP::PreprocessTargetUrl(const ParserJob &parser_job)
{
    return parser_job.last_url;
}

std::string ParserWP::GetFirstUrl(xmlNodePtr, const std::string &data_url)
{
    return data_url;
}

std::string ParserWP::GetNextUrl(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {

        BlackLibraryCommon::LogError(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return "";
    }
    current_node = body_seek.seek_node;

    const auto div_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=nav-next");
    if (!div_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed get next div seek for UUID: {}", uuid_);
        return "";
    }
    current_node = div_seek.seek_node;

    const auto a_seek = SeekToNodeByName(current_node, "a");
    if (!a_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed get next link seek for UUID: {}", uuid_);
        return "";
    }
    current_node = a_seek.seek_node;

    const auto link_seek = GetXmlAttributeContentByName(current_node, "href");
    if (!link_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get next nav link for UUID: {}", uuid_);
        return "";
    }

    return link_seek.result;
}

std::string ParserWP::GetSectionTitle(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {

        BlackLibraryCommon::LogError(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return "";
    }
    current_node = body_seek.seek_node;

    const auto title_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "h1",
        pattern_seek_t::XML_ATTRIBUTE, "class=entry-title");
    if (!title_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed get h1 section title seek for UUID: {}", uuid_);
        return "";
    }
    current_node = title_seek.seek_node;

    const auto title_seek_content = GetXmlNodeContent(current_node);
    if (!title_seek_content.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get title content for UUID: {}", uuid_);
        return "";
    }

    return title_seek_content.result;
}

time_t ParserWP::GetUpdateDate(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {

        BlackLibraryCommon::LogError(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return 0;
    }
    current_node = body_seek.seek_node;

    const auto time_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "time",
        pattern_seek_t::XML_ATTRIBUTE, "class=updated");
    if (!time_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed update time seek for UUID: {}", uuid_);
        return 0;
    }
    current_node = time_seek.seek_node;

    const auto time_content_result = GetXmlAttributeContentByName(current_node, "datetime");
    if (!time_content_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get time content for UUID: {}", uuid_);
        return 0;

    }
    struct tm tm;

    if (strptime(time_content_result.result.c_str(), "%Y-%m-%dT%H:%M:%S", &tm) == NULL)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to parse time content for UUID: {}", uuid_);
        return 0;
    }

    tm.tm_isdst = -1;
    return mktime(&tm);
}

} // namespace WP
} // namespace parsers
} // namespace core
} // namespace black_library
