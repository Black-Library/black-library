/**
 * TestIndexEntryParser.cc
 */

#include <iostream>

#include <FileOperations.h>
#include <LogOperations.h>
#include <TimeOperations.h>
#include <VersionOperations.h>

#include <TestIndexEntryParser.h>

namespace black_library {

namespace core {

namespace parsers {

namespace BlackLibraryCommon = black_library::core::common;

TestIndexEntryParser::TestIndexEntryParser(const njson &config) :
    IndexEntryParser(parser_t::ERROR_PARSER, config)
{
    source_name_ = BlackLibraryCommon::ERROR::source_name;
    source_url_ = BlackLibraryCommon::ERROR::source_url;
}

size_t TestIndexEntryParser::GetIndex()
{
    return index_;
}

size_t TestIndexEntryParser::GetTargetStartIndex()
{
    return target_start_index_;
}

size_t TestIndexEntryParser::GetTargetEndIndex()
{
    return target_end_index_;
}

int TestIndexEntryParser::TestCalculateIndexBounds(const ParserJob &parser_job)
{
    return CalculateIndexBounds(parser_job);
}

std::string TestIndexEntryParser::TestGetIndexEntryTitle(const ParserIndexEntry &index_entry)
{
    return GetIndexEntryTitle(index_entry);
}

ParserIndexEntry TestIndexEntryParser::TestExtractIndexEntry(xmlNodePtr root_node)
{
    return ExtractIndexEntry(root_node);
}

void TestIndexEntryParser::TestFindIndexEntries(xmlNodePtr root_node)
{
    FindIndexEntries(root_node);
}

void TestIndexEntryParser::TestFindMetaData(xmlNodePtr root_node)
{
    FindMetaData(root_node);
}

ParseSectionInfo TestIndexEntryParser::TestParseSection()
{
    return ParseSection();
}

ParserXmlNodeSeek TestIndexEntryParser::TestSeekToIndexEntryContent(xmlNodePtr root_node)
{
    return SeekToIndexEntryContent(root_node);
}

std::string TestIndexEntryParser::GetIndexEntryTitle(const ParserIndexEntry &index_entry)
{
    auto pos = index_entry.data_url.find_last_of("/");

    return index_entry.data_url.substr(pos + 1);
}

ParserIndexEntry TestIndexEntryParser::ExtractIndexEntry(xmlNodePtr root_node)
{
    (void) root_node;

    ParserIndexEntry index_entry;

    return index_entry;
}

void TestIndexEntryParser::FindIndexEntries(xmlNodePtr root_node)
{
    (void) root_node;
}

void TestIndexEntryParser::FindMetaData(xmlNodePtr root_node)
{
    (void) root_node;
}

ParseSectionInfo TestIndexEntryParser::ParseSection()
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
    const auto section_name = GetIndexEntryTitle(index_entry);

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

    auto version_check_result = db_adapter_->CheckVersion(section_content, uuid_, index_entry.index_num, index_entry.time_published, index_entry.data_url);

    if (version_check_result.has_error)
        return output;

    if (version_check_result.already_exists)
    {
        output.has_error = false;

        return output;
    }

    auto index_num = index_entry.index_num;

    // set index_num with offset if new
    // use offset if 
    // if (index_entry.data_url != md5_check.url)
    //     index_num += md5_index_num_offset_;

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

    output.has_error = false;

    return output;
}

ParserXmlNodeSeek TestIndexEntryParser::SeekToIndexEntryContent(xmlNodePtr root_node)
{
    (void) root_node;

    ParserXmlNodeSeek index_entry_content_seek;

    index_entry_content_seek.found = true;

    return index_entry_content_seek;
}

} // namespace parsers
} // namespace core
} // namespace black_library
