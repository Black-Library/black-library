/**
 * ParserCommon.cc
 */

#include <string.h>

#include <iomanip>
#include <iostream>
#include <cstdarg>
#include <sstream>

#include <SourceInformation.h>
#include <StringOperations.h>

#include <ParserCommon.h>

namespace black_library {

namespace core {

namespace parsers {

#define PARSE_SECTION_FILENAME_BUFFER_SIZE 128

namespace BlackLibraryCommon = black_library::core::common;

std::string GenerateXmlAttributeString(xmlNodePtr root_node)
{
    std::string attribute_content_string = "";
    xmlAttrPtr attribute = root_node->properties;
    while (attribute)
    {
        xmlChar *attr_content = xmlNodeListGetString(root_node->doc, attribute->children, 1);
        if (attribute->name !=NULL && attr_content != NULL)
        {
            attribute_content_string += std::string((char *) attribute->name) + ": " + std::string((char *) attr_content) + " ";
        }
        xmlFree(attr_content);
        attribute = attribute->next;
    }
    xmlFree(attribute);

    return attribute_content_string;
}

// based on http://www.xmlsoft.org/examples/tree1.c
std::string GenerateXmlDocTreeString(xmlNodePtr root_node)
{
    return GenerateXmlDocTreeStringHelper(root_node, 0);
}

std::string GenerateXmlDocTreeStringHelper(xmlNodePtr root_node, size_t depth)
{
    std::stringstream ss;
    xmlNodePtr cur_node = NULL;

    for (cur_node = root_node; cur_node; cur_node = cur_node->next)
    {
        std::string attribute_content_string = "";
        xmlAttrPtr attribute = cur_node->properties;
        while (attribute)
        {
            xmlChar *attr_content = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
            if (attribute->name !=NULL && attr_content != NULL)
            {
                attribute_content_string += std::string((char *) attribute->name) + ": " + std::string((char *) attr_content) + " ";
            }
            xmlFree(attr_content);
            attribute = attribute->next;
        }
        xmlFree(attribute);

        if (cur_node->type == XML_ELEMENT_NODE)
        {
            ss << GetSpaceString(depth) << "node type: Element, name: " << cur_node->name <<
            ", Attributes: " << attribute_content_string << std::endl;
        }
        else if (cur_node->type == XML_ATTRIBUTE_NODE)
        {
            ss << GetSpaceString(depth) << "node type: Attribute, name: " << cur_node->name <<
            ", Attributes: " << attribute_content_string << std::endl;
        }
        else if (cur_node->type == XML_TEXT_NODE)
        {
            std::string content_string = "unknown content";
            size_t string_size = 0;
            xmlChar *content = xmlNodeGetContent(cur_node);
            if (content != NULL)
            {
                std::string unclean = std::string((char *)content);
                content_string = BlackLibraryCommon::TrimWhitespace(unclean);
            }
            xmlFree(content);
            string_size = content_string.size();
            if (string_size > 0)
            {
                ss << GetSpaceString(depth) << "node type: Text, name: " << cur_node->name <<
                ", Attributes: " << attribute_content_string << ", Content: " << content_string << ", Size: " << string_size << std::endl;
            }
        }

        ss << GenerateXmlDocTreeStringHelper(cur_node->children, depth + 1);
    }

    return ss.str();
}

std::string GetParserBehaviorName(parser_behavior_t behavior)
{
    switch (behavior)
    {
    case parser_behavior_t::ERROR:
        return "ERROR_BEHAVIOR";
        break;
    case parser_behavior_t::INDEX_ENTRY:
        return "INDEX_ENTRY_BEHAVIOR";
        break;
    case parser_behavior_t::LINKED_LIST:
        return "LINKED_LIST_BEHAVIOR";
        break;
    default:
        return "NO_BEHAVIOR";
        break;
    }
}

std::string GetParserName(parser_t rep)
{
    switch (rep)
    {
    case parser_t::ERROR_PARSER:
        return "error_parser";
        break;
    case parser_t::AO3_PARSER:
        return "ao3_parser";
        break;
    case parser_t::FFN_PARSER:
        return "ffn_parser";
        break;
    case parser_t::RR_PARSER:
        return "rr_parser";
        break;
    case parser_t::SBF_PARSER:
        return "sbf_parser";
        break;
    case parser_t::SVF_PARSER:
        return "svf_parser";
        break;
    case parser_t::WP_PARSER:
        return "wp_parser";
        break;
    case parser_t::XF_PARSER:
        return "xf_parser";
        break;
    case parser_t::YT_PARSER:
        return "yt_parser";
        break;
    case parser_t::_NUM_PARSERS_TYPE:
        return "_num_parsers_type";
        break;
    default:
        return "no_parser";
        break;
    }
}

// c level string check is faster for smaller strings then
// needle.find(haystack) != std::string::npos
parser_t GetParserTypeByUrl(const std::string &url)
{
    parser_t rep = parser_t::ERROR_PARSER;

    if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::AO3::source_url))
    {
        rep = parser_t::AO3_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::FFN::source_url))
    {
        rep = parser_t::FFN_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::SBF::source_url))
    {
        rep = parser_t::SBF_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::SVF::source_url))
    {
        rep = parser_t::SVF_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::WP::source_url))
    {
        rep = parser_t::WP_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::RR::source_url))
    {
        rep = parser_t::RR_PARSER;
    }
    else if (BlackLibraryCommon::ContainsString(url, BlackLibraryCommon::YT::source_url))
    {
        rep = parser_t::YT_PARSER;
    }

    return rep;
}


// limited to 9999 sections right now (no CH10000), index starts at 1 for sections, max  version_num is 65535
std::string GetSectionFileName(const size_t &index_num, const std::string &section_name, const uint16_t &version_num)
{
    char buffer [PARSE_SECTION_FILENAME_BUFFER_SIZE];
    int res = snprintf(buffer, PARSE_SECTION_FILENAME_BUFFER_SIZE, "SEC%04lu_%s_VER%03d.html", index_num + 1, section_name.c_str(), version_num);
    if (res < 0)
        return "";

    return std::string(buffer);
}

std::string GetSpaceString(size_t num_tabs)
{
    std::string tab_string = "";

    for (size_t i = 0; i < num_tabs; ++i)
    {
        tab_string += "  ";
    }

    return tab_string;
}

std::string GetStatusName(job_status_t job_status)
{
    switch (job_status)
    {
    case job_status_t::JOB_ERROR:
        return "job_error";
        break;
    case job_status_t::JOB_MANAGER_QUEUED:
        return "job_manager_queued";
        break;
    case job_status_t::JOB_WORKER_QUEUED:
        return "job_worker_queued";
        break;
    case job_status_t::JOB_WORKING:
        return "job_working";
        break;
    case job_status_t::JOB_FINISHED:
        return "job_finished";
        break;
    default:
        return "job_error";
        break;
    }
}

ParserXmlNodeSeek SeekToNodeByElementAttr(xmlNodePtr root, std::string attr, std::string value)
{
    ParserXmlNodeSeek result;
    if (!xmlStrcmp(root->name, (const xmlChar *) "text") || !xmlStrcmp(root->name, (const xmlChar *) "style") ||
        !xmlStrcmp(root->name, (const xmlChar *) "script"))
    {
        return result;
    }

    xmlAttrPtr prop = root->properties;
    while (prop)
    {
        if (memcmp(prop->name, attr.c_str(), attr.length()) == 0 &&
            memcmp(prop->children->content, value.c_str(), value.length()) == 0)
        {
            result.seek_node = root;
            result.found = true;
            return result;
        }

        prop = prop->next;
    }

    xmlNodePtr current_node = root->children;

    while (current_node)
    {
        ParserXmlNodeSeek node_result = SeekToNodeByElementAttr(current_node, attr, value);

        if (node_result.found)
        {
            return node_result;
        }

        current_node = current_node->next;
    }

    return result;
}

ParserXmlContentResult GetXmlNodeContent(xmlNodePtr root_node)
{
    ParserXmlContentResult content_result;

    xmlChar *content = xmlNodeGetContent(root_node);
    if (content == NULL)
    {
        xmlFree(content);
        return content_result;
    }

    std::string unclean = std::string((char *)content);
    content_result.result = BlackLibraryCommon::TrimWhitespace(unclean);

    xmlFree(content);

    content_result.found = true;

    return content_result;
}

ParserXmlAttributeResult GetXmlAttributeContentByName(xmlNodePtr root_node, const std::string &target_name)
{
    ParserXmlAttributeResult attr_result;

    xmlAttrPtr attribute = root_node->properties;
    xmlChar *attr_content = NULL;

    while (attribute)
    {
        const xmlChar *attr_name = attribute->name;
        if (attr_name)
        {
            if (!target_name.compare(std::string((char *)attr_name)))
            {
                attr_content = xmlNodeListGetString(root_node->doc, attribute->children, 1);
                if (attr_content != NULL)
                {
                    attr_result.result = std::string((char *) attr_content);
                    attr_result.found = true;
                }
                break;
            }
        }
        attribute = attribute->next;
    }
    xmlFree(attr_content);

    return attr_result;
}

bool NodeHasAttribute(xmlNodePtr root_node, const std::string &target_name)
{
    xmlAttrPtr attribute = root_node->properties;
    bool found = false;

    while (attribute)
    {
        const xmlChar *attr_name = attribute->name;
        if (attr_name != NULL)
        {
            if (!target_name.compare(std::string((char *)attr_name)))
            {
                found = true;
                break;
            }
        }

        attribute = attribute->next;
    }

    return found;
}

bool NodeHasAttributeContent(xmlNodePtr root_node, const std::string &target_content)
{
    xmlAttrPtr attribute = root_node->properties;
    xmlChar *attr_content = NULL;
    bool found = false;

    while (attribute)
    {
        attr_content = xmlNodeListGetString(root_node->doc, attribute->children, 1);
        if (attr_content != NULL)
        {
            std::string compare = std::string((char *)attr_content);
            // std::cout << "attr_content: " << compare << " - target: " << target_content << std::endl;
            if (!compare.compare(0, target_content.size(), target_content))
                found = true;
        }
        xmlFree(attr_content);

        attribute = attribute->next;
    }
    xmlFree(attribute);

    return found;
}

ParserXmlNodeSeek SeekToNodeByName(xmlNodePtr root_node, const std::string &name)
{
    ParserXmlNodeSeek seek;
    xmlNodePtr current_node = NULL;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        if (!xmlStrcmp(current_node->name, (const xmlChar *) name.c_str()))
        {
            seek.seek_node = current_node;
            seek.found = true;
            break;
        }
    }

    return seek;
}

ParserXmlNodeSeek SeekToNodeByNameRecursive(xmlNodePtr root_node, const std::string &name)
{
    ParserXmlNodeSeek seek;
    xmlNodePtr current_node = NULL;

    for (current_node = root_node; current_node; current_node = current_node->next)
    {
        if (current_node->type != XML_ELEMENT_NODE)
            continue;

        if (!xmlStrcmp(current_node->name, (const xmlChar *) name.c_str()))
        {
            seek.seek_node = current_node;
            seek.found = true;
            return seek;
        }

        ParserXmlNodeSeek children_seek = SeekToNodeByNameRecursive(current_node->children, name);

        if (children_seek.found)
        {
            seek.seek_node = children_seek.seek_node;
            seek.found = true;
            return seek;
        }
    }

    return seek;
}

bool SeekToNodeByPatternHelper(xmlNodePtr root_node)
{
    (void) root_node;
    return true;
}

std::ostream& operator<<(std::ostream& out, const pattern_seek_t value){
    static std::unordered_map<pattern_seek_t, std::string> strings;
    strings.emplace(pattern_seek_t::XML_NAME, "XML_NAME");
    strings.emplace(pattern_seek_t::XML_ATTRIBUTE, "XML_ATTRIBUTE");
    strings.emplace(pattern_seek_t::XML_CONTENT, "XML_CONTENT");

    return out << strings[value];
}

} // namespace parsers
} // namespace core
} // namespace black_library
