/**
 * ParserYT.cc
 */

#include <iostream>
#include <sstream>

#include <FileOperations.h>
#include <TimeOperations.h>

#include <ParserYT.h>

namespace black_library {

namespace core {

namespace parsers {

namespace YT {

namespace BlackLibraryCommon = black_library::core::common;

static constexpr const char YoutubeDlBaseString[] = "youtube-dl --no-overwrites --restrict-filenames --write-description --write-info-json --add-metadata ";

ParserYT::ParserYT(const njson &config) :
    IndexEntryParser(parser_t::YT_PARSER, config)
{
    source_name_ = BlackLibraryCommon::YT::source_name;
    source_url_ = BlackLibraryCommon::YT::source_url;

    is_playlist = false;
}

ParserIndexEntry ParserYT::ExtractIndexEntry(xmlNodePtr root_node)
{
    ParserIndexEntry index_entry;

    const auto url_seek = SeekToNodeByPattern(root_node, pattern_seek_t::XML_NAME, "meta",
        pattern_seek_t::XML_ATTRIBUTE, "property=og:url");

    if (url_seek.found)
    {
        const auto url_content_result = GetXmlAttributeContentByName(url_seek.seek_node, "content");
        if (url_content_result.found)
        {
            index_entry.data_url = BlackLibraryCommon::SubstringAfterString(url_content_result.result, source_url_);
            index_entry.index_num = 0;
        }
    }

    return index_entry;
}

void ParserYT::FindIndexEntries(xmlNodePtr root_node)
{
    const auto index_entry = ExtractIndexEntry(root_node);

    index_entries_.emplace_back(index_entry);
}

void ParserYT::FindMetaData(xmlNodePtr root_node)
{
    xmlNodePtr current_node = NULL;

    const auto body_seek = SeekToNodeByName(root_node, "body");
    if (!body_seek.found)
    {
        std::cout << "Error: failed body seek" << std::endl;
        return;
    }

    current_node = body_seek.seek_node;

    ParserXmlNodeSeek title_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "meta",
        pattern_seek_t::XML_ATTRIBUTE, "itemprop=name");
    if (title_seek.found)
    {
        ParserXmlAttributeResult content_result = GetXmlAttributeContentByName(title_seek.seek_node, "content");
        if (content_result.found)
            title_ = content_result.result;
    }

    ParserXmlNodeSeek author_seek = SeekToNodeByPattern(current_node, pattern_seek_t::XML_NAME, "span",
        pattern_seek_t::XML_ATTRIBUTE, "itemprop=author");
    if (author_seek.found)
    {
        ParserXmlNodeSeek name_seek = SeekToNodeByPattern(author_seek.seek_node->children, pattern_seek_t::XML_NAME, "link",
        pattern_seek_t::XML_ATTRIBUTE, "itemprop=name");
        if (name_seek.found)
        {
            ParserXmlAttributeResult content_result = GetXmlAttributeContentByName(name_seek.seek_node, "content");
            if (content_result.found)
            {
                author_ = content_result.result;
            }
        }
    }
}

ParseSectionInfo ParserYT::ParseSection()
{
    ParseSectionInfo output;
    const auto index_entry = index_entries_[index_];

    const auto index_entry_url = "https://www." + source_url_ + index_entry.data_url;
    std::cout << GetParserName(parser_type_) << " ParseSection: " << GetParserBehaviorName(parser_behavior_) << " - parse url: " << index_entry_url << " - " << index_entry.name << std::endl;

    std::stringstream ss;
    int ret = 0;

    // get audio
    ss << YoutubeDlBaseString << "--extract-audio ";
    if (is_playlist)
        ss << "--yes-playlist ";
    ss << "--output '" << local_des_ << "%(title)s.%(ext)s' " << index_entry_url;

    std::cout << ss.str() << std::endl;

    ret = system(ss.str().c_str());

    if (ret < 0)
        return output;

    // clear stringstream
    ss.str(std::string());

    // get video
    ss << YoutubeDlBaseString;
    if (is_playlist)
        ss << "--yes-playlist ";
    ss << "--output '" << local_des_ << "%(title)s.%(ext)s' " << index_entry_url;

    std::cout << ss.str() << std::endl;

    ret = system(ss.str().c_str());

    if (ret < 0)
        return output;

    output.has_error = false;

    return output;
}

std::string ParserYT::PreprocessTargetUrl(const ParserJob &parser_job)
{
    return parser_job.url;
}

std::string ParserYT::GetYTIndexEntryTitle(const ParserIndexEntry &index_entry)
{
    auto pos = index_entry.data_url.find_last_of("/");

    return index_entry.data_url.substr(pos + 1);
}

ParserXmlNodeSeek ParserYT::SeekToIndexEntryContent(xmlNodePtr root_node)
{
    ParserXmlNodeSeek index_entry_content_seek;
    
    (void) root_node;

    return index_entry_content_seek;
}

} // namespace YT
} // namespace parsers
} // namespace core
} // namespace black_library
