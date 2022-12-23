/**
 * ParserAO3.cc
 */

#include <iostream>

#include <LogOperations.h>
#include <TimeOperations.h>

#include <ParserAO3.h>

namespace black_library {

namespace core {

namespace parsers {

namespace AO3 {

namespace BlackLibraryCommon = black_library::core::common;

ParserAO3::ParserAO3(const njson &config) :
    IndexEntryParser(parser_t::AO3_PARSER, config)
{
    source_name_ = BlackLibraryCommon::AO3::source_name;
    source_url_ = BlackLibraryCommon::AO3::source_url;
}

ParserAO3::~ParserAO3()
{
    done_ = true;
}

ParserIndexEntry ParserAO3::ExtractIndexEntry(xmlNodePtr root_node)
{
    (void) root_node;
    ParserIndexEntry index_entry;
    return index_entry;
}

void ParserAO3::FindIndexEntries(xmlNodePtr root_node)
{
    ParserIndexEntry index_entry;

    index_entry.data_url = target_url_;
    index_entry.name = title_;

    ParserTimeResult time_result = GetPublishedTime(root_node);

    if (time_result.found)
    {
        index_entry.time_published = time_result.time;
    }

    index_entry.index_num = 0;

    index_entries_.emplace_back(index_entry);
}

void ParserAO3::FindMetaData(xmlNodePtr root_node)
{
    ParserXmlNodeSeek title_result = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "h2",
        pattern_seek_t::XML_ATTRIBUTE, "class=title heading");
    if (title_result.found)
    {
        ParserXmlContentResult t_result = GetXmlNodeContent(title_result.seek_node);
        if (t_result.found)
        {
            title_ = t_result.result;
        }
    }

    ParserXmlNodeSeek author_result = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "a",
        pattern_seek_t::XML_ATTRIBUTE, "rel=author");
    if (author_result.found)
    {
        ParserXmlContentResult a_result = GetXmlNodeContent(author_result.seek_node);
        if (a_result.found)
        {
            author_ = a_result.result;
        }
    }
}

ParseSectionInfo ParserAO3::ParseSection()
{
    ParseSectionInfo output;
    const auto index_entry = index_entries_[index_];

    const auto url_adult = index_entry.data_url;
    const auto index_entry_curl_result = CurlRequest(url_adult);

    xmlDocPtr section_doc_tree = htmlReadDoc((xmlChar*) index_entry_curl_result.c_str(), NULL, NULL,
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!section_doc_tree)
    {
        BlackLibraryCommon::LogError(parser_name_, "Unable to parse into doc_tree");
        output.has_error = true;
        return output;
    }

    xmlNodePtr root_node = xmlDocGetRootElement(section_doc_tree);
    xmlNodePtr next = root_node->children;

    while (memcmp(next->name, "body", 4))
    {
        next = next->next;
    }

    ParserXmlNodeSeek workskin_result = SeekToNodeByPattern(next, pattern_seek_t::XML_ATTRIBUTE, "id=workskin");

    if (!workskin_result.found)
    {
        BlackLibraryCommon::LogError(parser_name_, "workskin seek failure");
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    xmlNodePtr workskin = workskin_result.seek_node;
    xmlDocSetRootElement(section_doc_tree, workskin);

    ParserXmlNodeSeek words_result = SeekToNodeByPattern(root_node,
        pattern_seek_t::XML_NAME, "dd", pattern_seek_t::XML_ATTRIBUTE, "class=words");
    if (!words_result.found)
    {
        BlackLibraryCommon::LogWarn(parser_name_, "Could not get work length");
    }
    else
    {
        ParserXmlContentResult w_result = GetXmlNodeContent(words_result.seek_node);
        if (w_result.found && strcmp(w_result.result.c_str(), ""))
        {
            std::cout << w_result.result << std::endl;
            output.length = (size_t) std::stoi(w_result.result) / 1000;
        }
    }

    const auto section_file_name = GetSectionFileName(index_entry.index_num, title_, 0);

    FILE* section_output_file;
    std::string file_path = local_des_ + section_file_name;
    BlackLibraryCommon::LogDebug(parser_name_, "FILEPATH: {}", file_path);
    section_output_file = fopen(file_path.c_str(), "w+");

    if (section_output_file == NULL)
    {
        BlackLibraryCommon::LogError(parser_name_, "Failed to open file with path: {}", file_path);
        xmlFreeDoc(section_doc_tree);
        return output;
    }

    xmlElemDump(section_output_file, section_doc_tree, workskin_result.seek_node);
    // TODO: figure out how to handle seg faults/other errors in threadpool/thread
    fclose(section_output_file);

    xmlFreeDoc(section_doc_tree);

    output.has_error = false;

    return output;
}

std::string ParserAO3::PreprocessTargetUrl(const ParserJob &parser_job)
{
    return parser_job.url + "?view_full_work=true&view_adult=true";
}

ParserTimeResult ParserAO3::GetPublishedTime(xmlNodePtr root_node)
{
    ParserTimeResult result;

    ParserXmlNodeSeek meta_result = SeekToNodeByElementAttr(root_node, "role", "complementary");
    if (!meta_result.found)
    {
        return result;
    }

    ParserXmlNodeSeek stats_result = SeekToNodeByPattern(meta_result.seek_node,
        pattern_seek_t::XML_NAME, "dd", pattern_seek_t::XML_ATTRIBUTE, "class=stats");
    if (!stats_result.found)
    {
        return result;
    }

    ParserXmlNodeSeek published_result = SeekToNodeByPattern(stats_result.seek_node,
        pattern_seek_t::XML_NAME, "dd", pattern_seek_t::XML_ATTRIBUTE, "class=published");
    if (!published_result.found)
    {
        return result;
    }

    ParserXmlContentResult p_result = GetXmlNodeContent(published_result.seek_node);
    if (!p_result.found)
    {
        return result;
    }

    result.time = BlackLibraryCommon::ParseTimet("%F", p_result.result);
    result.found = true;

    return result;
}
} // namespace AO3

} // namespace parsers
} // namespace core
} // namespace black_library
