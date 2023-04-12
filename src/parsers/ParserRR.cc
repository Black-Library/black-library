/**
 * ParserRR.cc
 */

#include <iostream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <TimeOperations.h>
#include <VersionOperations.h>

#include <ParserRR.h>

namespace black_library {

namespace core {

namespace parsers {

namespace RR {

namespace BlackLibraryCommon = black_library::core::common;

ParserRR::ParserRR(const njson &config) :
    IndexEntryParser(parser_t::RR_PARSER, config)
{
    source_name_ = BlackLibraryCommon::RR::source_name;
    source_url_ = BlackLibraryCommon::RR::source_url;
}

std::string ParserRR::GetRRIndexEntryTitle(const ParserIndexEntry &index_entry)
{
    auto pos = index_entry.data_url.find_last_of("/");

    return index_entry.data_url.substr(pos + 1);
}

ParserIndexEntry ParserRR::ExtractIndexEntry(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;
    ParserIndexEntry index_entry;

    auto td_seek = SeekToNodeByName(root_node->children, "td");

    if (!td_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry td seek for UUID: {}", uuid_);
        return index_entry;
    }

    current_node = td_seek.seek_node;

    auto a_seek = SeekToNodeByName(current_node->children, "a");

    if (!a_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry a seek for UUID: {}", uuid_);
        return index_entry;
    }

    current_node = a_seek.seek_node;

    if (!NodeHasAttribute(current_node, "href"))
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry hrf seek for UUID: {}", uuid_);
        return index_entry;
    }

    auto url_result = GetXmlAttributeContentByName(current_node, "href");

    if (!url_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry url result for UUID: {}", uuid_);
        return index_entry;
    }

    index_entry.data_url = "https://" + source_url_ + url_result.result;

    current_node = current_node->children;

    auto index_entry_name_result = GetXmlNodeContent(current_node);

    if (!index_entry_name_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry name content result for UUID: {}", uuid_);
        return index_entry;
    }

    index_entry.name = index_entry_name_result.result;

    auto time_seek = SeekToNodeByNameRecursive(root_node, "time");

    if (!time_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry time seek for UUID: {}", uuid_);
        return index_entry;
    }

    auto time_attr_seek = GetXmlAttributeContentByName(time_seek.seek_node, "unixtime");

    if (!time_attr_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed index entry time attribute seek for UUID: {}", uuid_);
        return index_entry;
    }

    BlackLibraryCommon::LogTrace(parser_name_, "time attribute seek result for UUID: {} : {}", uuid_, time_attr_seek.result);

    index_entry.time_published = std::stoll(time_attr_seek.result);

    BlackLibraryCommon::LogTrace(parser_name_, "index entry time for UUID: {} : {}", uuid_, index_entry.time_published);

    return index_entry;
}

void ParserRR::FindIndexEntries(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    ParserXmlNodeSeek body_seek = SeekToNodeByName(root_node, "body");

    if (!body_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed body seek for UUID: {}", uuid_);
        return;
    }

    current_node = body_seek.seek_node->children;

    ParserXmlNodeSeek tbody_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "tbody");
    if (tbody_seek.found)
    {
        xmlNodePtr index_node = NULL;
        size_t index_num = 0;
        for (index_node = tbody_seek.seek_node->children; index_node; index_node = index_node->next)
        {
            if (!xmlStrcmp(index_node->name, (const xmlChar *) "tr"))
            {
                ParserIndexEntry index_entry = ExtractIndexEntry(index_node);
                if (index_entry.data_url.empty())
                    continue;

                index_entry.index_num = index_num;
                index_entries_.emplace_back(index_entry);
                ++index_num;
            }
        }

        xmlFree(index_node);
    }
}

void ParserRR::FindMetaData(xmlNodePtr root_node)
{
    ParserXmlNodeSeek head_seek = SeekToNodeByName(root_node, "head");

    if (!head_seek.found)
    {
        BlackLibraryCommon::LogWarn(parser_name_, "Failed to get metadata for UUID: {}", uuid_);
        return;
    }

    xmlNodePtr current_node = NULL;

    for (current_node = head_seek.seek_node->children; current_node; current_node = current_node->next)
    {
        if (!xmlStrcmp(current_node->name, (const xmlChar *) "meta"))
        {
            auto property_result = GetXmlAttributeContentByName(current_node, "property");

            if (!property_result.found)
                continue;

            if (property_result.result == "books:author")
            {
                auto author_result = GetXmlAttributeContentByName(current_node, "content");

                if (!author_result.found)
                    continue;

                author_ = author_result.result;
            }
            else if (property_result.result == "og:url")
            {
                auto title_result = GetXmlAttributeContentByName(current_node, "content");

                if (!title_result.found)
                    continue;

                std::string unprocessed_title = title_result.result;
                size_t found = unprocessed_title.find_last_of("/\\");
                title_ = unprocessed_title.substr(found + 1, unprocessed_title.size());
            }
        }
    }

    xmlFree(current_node);
}

ParseSectionInfo ParserRR::ParseSection()
{
    ParseSectionInfo output;
    if (index_entry_queue_.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to get front of queue for UUID: {}", uuid_);
        return output;
    }

    const auto index_entry = index_entry_queue_.front();

    BlackLibraryCommon::LogDebug(parser_name_, "ParseSection: {} section_url: {} - {}", GetParserBehaviorName(parser_behavior_), index_entry.data_url, index_entry.name);

    const auto curl_request_result = CurlRequest(index_entry.data_url);
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

    ParserXmlNodeSeek index_entry_content_seek = SeekToIndexEntryContent(current_node);
    if (!index_entry_content_seek.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed content seek for UUID: {}", uuid_);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    current_node = index_entry_content_seek.seek_node;

    for (length_node = current_node->children; length_node; length_node = length_node->next)
    {
        if (length_node->type != XML_ELEMENT_NODE)
            continue;
        ++length;
    }

    output.length = length;

    // generate section file name from index entry
    const auto section_name = GetRRIndexEntryTitle(index_entry);

    const auto sanatized_section_name = BlackLibraryCommon::SanitizeFileName(section_name);

    if (sanatized_section_name.empty())
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to generate index entry title name from: {}", section_name);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    // dump content and free afterwards
    auto section_content = SectionDumpContent(section_doc_tree, current_node);
    xmlFreeDoc(section_doc_tree);

    if (section_content.empty())
        return output;

    // version check
    auto index_num = index_entry.index_num;
    auto section_md5 = BlackLibraryCommon::GetMD5Hash(section_content);
    BlackLibraryCommon::LogDebug(parser_name_, "Section UUID: {} index: {} checksum hash: {}", uuid_, index_num, section_md5);

    BlackLibraryCommon::Md5Sum md5_check;
    if (md5_check_callback_)
        md5_check = md5_check_callback_(section_md5, uuid_);

    // md5 already exists with correct md5_sum, update md5 with the correct index_num
    if (md5_check.md5_sum != BlackLibraryCommon::EmptyMD5Version)
    {
        BlackLibraryCommon::LogDebug(parser_name_, "Version hash matches: {} index: {}, skip file save", uuid_, md5_check.index_num);
        index_num = md5_check.index_num;

        if (md5_update_callback_)
            md5_update_callback_(uuid_, index_num, section_md5, index_entry.time_published, index_entry.data_url, 0);

        output.has_error = false;

        return output;
    }

    if (md5_read_callback_)
        md5_check = md5_read_callback_(uuid_, index_entry.data_url);

    // set index_num if new, other wise just update
    if (index_entry.data_url != md5_check.url)
        index_num += md5_index_num_offset_;

    uint16_t version_num = 0;
    if (version_read_num_callback_)
        version_num = version_read_num_callback_(uuid_, index_num);

    // save file
    const auto section_file_name = GetSectionFileName(index_num, sanatized_section_name, version_num);

    if (SectionFileSave(section_content, section_file_name))
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed section file save with UUID: {}", uuid_);
        return output;
    }

    if (md5_update_callback_)
        md5_update_callback_(uuid_, index_num, section_md5, index_entry.time_published, index_entry.data_url, version_num);

    output.has_error = false;

    return output;
}

ParserXmlNodeSeek ParserRR::SeekToIndexEntryContent(xmlNodePtr root_node)
{
    ParserXmlNodeSeek index_entry_content_seek;
    xmlNodePtr current_node = NULL;
    bool found = false;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        if (NodeHasAttributeContent(current_node, "chapter-inner chapter-content"))
        {
            index_entry_content_seek.seek_node = current_node;
            found = true;
            break;
        }

        ParserXmlNodeSeek children_seek = SeekToIndexEntryContent(current_node->children);

        if (children_seek.seek_node != NULL)
            index_entry_content_seek.seek_node = children_seek.seek_node;

        found = found || children_seek.found;
    }

    index_entry_content_seek.found = found;

    return index_entry_content_seek;
}

} // namespace RR
} // namespace parsers
} // namespace core
} // namespace black_library
