/**
 * ParserXF.cc
 */

#include <chrono>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>

#include <FileOperations.h>
#include <LogOperations.h>
#include <VersionOperations.h>

#include <ParserXF.h>

namespace black_library {

namespace core {

namespace parsers {

namespace XF {

namespace BlackLibraryCommon = black_library::core::common;

ParserXF::ParserXF(parser_t parser_type, const njson &config) :
    LinkedListParser(parser_type, config)
{
    source_name_ = BlackLibraryCommon::ERROR::source_name;
    source_url_ = BlackLibraryCommon::ERROR::source_url;
}

void ParserXF::FindMetaData(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {

        BlackLibraryCommon::LogWarn(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return;
    }

    current_node = body_seek.seek_node;

    const auto threadmark_seek = SeekToThreadmark(current_node);
    if (!threadmark_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to find threadmark for UUID: {}", uuid_);
        return;
    }

    current_node = threadmark_seek.seek_node;

    const auto message_inner_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=message-inner");
    if (!message_inner_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed message-inner seek for UUID: {}", uuid_);
        return;
    }

    current_node = message_inner_seek.seek_node;

    const auto message_userdetails_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=message-userDetails");
    if (!message_userdetails_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed message-userDetails seek for UUID: {}", uuid_);
        return;
    }

    current_node = message_userdetails_seek.seek_node->children;

    const auto name_seek = SeekToNodeByName(current_node, "h4");
    if (!name_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed author content seek for UUID: {}", uuid_);
        return;
    }

    current_node = name_seek.seek_node;

    const auto author_result = GetXmlNodeContent(current_node->children);
    if (!author_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get author content for UUID: {}", uuid_);
        return;
    }

    author_ = author_result.result;
}

ParseSectionInfo ParserXF::ParseSection()
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

    const auto target_post = GetTargetPost(working_url);

    const auto p_body_main_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=p-body-main  ");
    if (!p_body_main_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed p-body-main seek for UUID: {}", uuid_);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    current_node = p_body_main_seek.seek_node;

    const auto section_post_seek = SeekToSectionPost(current_node, target_post);
    if (!section_post_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed post seek for UUID: {}", uuid_);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    current_node = section_post_seek.seek_node;

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
    if (working_index < target_start_index_)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "Target start index: {} - current index: {} skipping file save", target_start_index_, working_index);
        output.has_error = false;
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    // reset current node to threadmark header
    current_node = root_node->children;

    const auto section_content_seek = SeekToSectionContent(current_node, target_post);
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

    const auto section_name = GetXFTitle(section_title);

    const auto sanatized_section_name = BlackLibraryCommon::SanitizeFileName(section_name);

    if (sanatized_section_name.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to generate section name from: {}", section_name);
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

std::string ParserXF::PreprocessTargetUrl(const ParserJob &parser_job)
{
    title_ = GetWorkTitleFromUrl(parser_job.url);

    if (parser_job.url == parser_job.last_url)
    {
        return parser_job.url;
    }

    return parser_job.last_url;
}

std::string ParserXF::GetFirstUrl(xmlNodePtr root_node, const std::string &data_url)
{
    xmlNodePtr current_node = NULL;

    const auto threadmark_seek = SeekToThreadmark(root_node);
    if (!threadmark_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed threadmark seek from url: {}", data_url);
        return "";
    }

    current_node = threadmark_seek.seek_node->children->next;

    const auto post_id_result = GetXmlAttributeContentByName(current_node, "id");
    if (!post_id_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to get post id from url: {}", data_url);
        return "";
    }

    return data_url + '#' + post_id_result.result;
}

std::string ParserXF::GetNextUrl(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;
    std::string next_url = "";

    const auto threadmark_nav_seek = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "li",
        pattern_seek_t::XML_ATTRIBUTE, "class=threadmark-nav");
    if (threadmark_nav_seek.found)
    {
        bool found_next = false;

        for (current_node = threadmark_nav_seek.seek_node->children; current_node; current_node = current_node->next)
        {
            if (!NodeHasAttributeContent(current_node, "threadmark-control threadmark-control--next"))
                continue;

            const auto href_result = GetXmlAttributeContentByName(current_node, "href");

            if (!href_result.found)
                continue;

            found_next = true;
            next_url = href_result.result;

            break;
        }

        if (!found_next)
        {
            BlackLibraryCommon::LogDebug(parser_name_, "Reached end of linked list for UUID: {}", uuid_);
            reached_end_ = true;
            return next_url_;
        }
    }

    return "https://" + source_url_ + next_url;
}

std::string ParserXF::GetSectionTitle(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    const auto section_threadmark_header_seek = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "div",
        pattern_seek_t::XML_ATTRIBUTE, "class=message-cell message-cell--threadmark-header");
    if (!section_threadmark_header_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed threadmark header seek for UUID: {}", uuid_);
        return "";
    }

    current_node = section_threadmark_header_seek.seek_node->children;

    const auto span_seek_0 = SeekToNodeByName(current_node, "span");

    if (!span_seek_0.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to find span node for UUID: {}", uuid_);
        return "";
    }

    current_node = span_seek_0.seek_node->children;

    const auto span_seek_1 = SeekToNodeByName(current_node, "span");

    if (!span_seek_1.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to find span node for UUID: {}", uuid_);
        return "";
    }

    current_node = span_seek_1.seek_node;

    const auto title_result = GetXmlNodeContent(current_node->children);

    if (!title_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to find title content for UUID: {}", uuid_);
        return "";
    }

    return title_result.result;
}

std::string ParserXF::GetTargetPost(const std::string &data_url)
{
    auto pos = data_url.find_last_of("-");
    const auto target_id = data_url.substr(pos + 1);

    return "post-" + target_id;
}

time_t ParserXF::GetUpdateDate(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    const auto message_attribution_main_seek = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "ul",
        pattern_seek_t::XML_ATTRIBUTE, "class=message-attribution-main listInline ");
    if (!message_attribution_main_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed message attribution main seek for UUID: {}", uuid_);
        return 0;
    }

    current_node = message_attribution_main_seek.seek_node->children;

    const auto li_seek = SeekToNodeByName(current_node, "li");
    if (!li_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed 'li' seek for UUID: {}", uuid_);
        return 0;
    }

    current_node = li_seek.seek_node->children;

    const auto a_seek = SeekToNodeByName(current_node, "a");
    if (!a_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed 'a' seek for UUID: {}", uuid_);
        return 0;
    }

    current_node = a_seek.seek_node->children;

    const auto time_seek = SeekToNodeByName(current_node, "time");
    if (!time_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed time seek for UUID: {}", uuid_);
        return 0;
    }

    current_node = time_seek.seek_node;

    const auto time_result = GetXmlAttributeContentByName(current_node, "data-time");

    if (!time_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed time result for UUID: {}", uuid_);
        return 0;
    }

    return std::stol(time_result.result);
}

std::string ParserXF::GetWorkTitleFromUrl(const std::string &data_url)
{
    std::string unprocessed_title = data_url;
    size_t found_0 = unprocessed_title.find_last_of(".");
    std::string removed_threadmarks = unprocessed_title.substr(0, found_0);
    size_t found_1 = removed_threadmarks.find_last_of("/\\");

    auto processed_title = removed_threadmarks.substr(found_1 + 1, removed_threadmarks.size());

    BlackLibraryCommon::SanatizeString(processed_title);

    return processed_title;
}

std::string ParserXF::GetXFTitle(const std::string &title)
{
    std::locale loc;
    std::string xf_title_name = title;

    std::transform(xf_title_name.begin(), xf_title_name.end(), xf_title_name.begin(),
    [&loc](char ch)
    {
        return ch == ' ' ? '-' : std::tolower(ch, loc);
    });

    // sanatize removes spaces
    BlackLibraryCommon::SanatizeString(xf_title_name);

    return xf_title_name;
}

ParserXmlNodeSeek ParserXF::SeekToSectionContent(xmlNodePtr root_node, const std::string &target_post)
{
    ParserXmlNodeSeek section_content_seek;
    xmlNodePtr current_node = NULL;
    bool found = false;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        if (NodeHasAttribute(current_node, "data-lb-id"))
        {
            auto post_id = GetXmlAttributeContentByName(current_node, "data-lb-id");
            if (!post_id.found)
                continue;

            // find by post_id
            if (!post_id.result.compare(target_post))
            {
                auto inner_seek = SeekToNodeByNameRecursive(current_node->children, "div");
                if (!inner_seek.found)
                    continue;

                section_content_seek.seek_node = inner_seek.seek_node;
                found = true;
                break;
            }
        }

        ParserXmlNodeSeek children_seek = SeekToSectionContent(current_node->children, target_post);

        if (children_seek.seek_node != NULL)
            section_content_seek.seek_node = children_seek.seek_node;

        found = found || children_seek.found;
    }

    section_content_seek.found = found;

    return section_content_seek;
}

ParserXmlNodeSeek ParserXF::SeekToSectionPost(xmlNodePtr root_node, const std::string &target_post)
{
    ParserXmlNodeSeek section_post_seek;
    xmlNodePtr current_node = NULL;
    bool found = false;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        if (NodeHasAttribute(current_node, "data-content"))
        {
            if (NodeHasAttributeContent(current_node, target_post))
            {
                section_post_seek.seek_node = current_node;
                found = true;
                break;
            }
        }

        ParserXmlNodeSeek children_seek = SeekToSectionPost(current_node->children, target_post);

        if (children_seek.seek_node != NULL)
            section_post_seek.seek_node = children_seek.seek_node;

        found = found || children_seek.found;
    }

    section_post_seek.found = found;

    return section_post_seek;
}

ParserXmlNodeSeek ParserXF::SeekToThreadmark(xmlNodePtr root_node)
{
    ParserXmlNodeSeek threadmark_node_seek;

    xmlNodePtr current_node = NULL;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        ParserXmlNodeSeek children_seek = SeekToThreadmark(current_node->children);
        if (children_seek.found)
        {
            threadmark_node_seek.seek_node = children_seek.seek_node;
            threadmark_node_seek.found = true;
            return threadmark_node_seek;
        }

        if (NodeHasAttributeContent(current_node, "message message--post hasThreadmark  js-post js-inlineModContainer   "))
        {
            const auto threadmark_header_seek = SeekToNodeByPattern(current_node->children, pattern_seek_t::XML_NAME, "div",
                pattern_seek_t::XML_ATTRIBUTE, "class=message-cell message-cell--threadmark-header");
            if (!threadmark_header_seek.found)
            {
                BlackLibraryCommon::LogError(parser_name_, "Failed to find threadmark header for UUID: {}", uuid_);
                continue;
            }

            const auto label_seek = SeekToNodeByPattern(threadmark_header_seek.seek_node->children, pattern_seek_t::XML_NAME, "label");
            if (!label_seek.found)
            {
                BlackLibraryCommon::LogError(parser_name_, "Failed to find label for UUID: {}", uuid_);
                continue;
            }
            
            const auto label_content = GetXmlNodeContent(label_seek.seek_node);

            if (label_content.result == "Threadmarks")
            {
                threadmark_node_seek.seek_node = current_node;
                threadmark_node_seek.found = true;
                return threadmark_node_seek;
            }
        }
    }

    return threadmark_node_seek;
}

} // namespace XF
} // namespace parsers
} // namespace core
} // namespace black_library
