/**
 * Parser.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_H__

#include <string.h>

#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>

#include <ConfigOperations.h>
#include <SourceInformation.h>

#include "ParserCommon.h"
#include "ParserDbAdapter.h"
#include "ParserTimeGenerator.h"

namespace black_library {

namespace core {

namespace parsers {

class Parser
{
public:
    Parser(parser_t parser_type, const njson &config);
    // Parser() : Parser(parser_t::ERROR_PARSER, some base config?){}; TODO
    Parser(const Parser &parser);
    virtual ~Parser();

    virtual ParserResult Parse(const ParserJob &parser_job);
    void Stop();

    std::string CurlRequest(const std::string &url);

    void SetLocalFilePath(const std::string &local_des);

    bool GetDone();
    parser_t GetParserType();
    parser_behavior_t GetParserBehaviorType();
    std::string GetSourceName();
    std::string GetSourceUrl();

    int RegisterProgressNumberCallback(const progress_number_callback &callback);
    int RegisterVersionReadNumCallback(const version_read_num_callback &callback);

    int SetDbAdapter(const std::shared_ptr<ParserDbAdapter> &db_adapter);

protected:
    std::string SectionDumpContent(const xmlDocPtr doc_ptr, const xmlNodePtr node_ptr);
    bool SectionFileSave(const std::string &section_content, const std::string &section_file_name);
    virtual int CalculateIndexBounds(const ParserJob &parser_job);
    virtual void ExpendedAttempts();
    virtual void FindMetaData(xmlNodePtr root_node);
    virtual void IndicateNextSection();
    virtual void ParseLoop(ParserResult &parser_result);
    virtual ParseSectionInfo ParseSection();
    virtual void PostParseLoop(ParserResult &parser_result);
    virtual int PreParseLoop(xmlNodePtr root_node, const ParserJob &parser_job);
    virtual std::string PreprocessTargetUrl(const ParserJob &parser_job);
    virtual bool ReachedEnd();
    virtual void SaveLastUrl(ParserResult &parser_result);
    virtual void SaveMetaData(ParserResult &parser_result);
    virtual void SaveUpdateDate(ParserResult &parser_result);

    std::vector<BlackLibraryCommon::Md5Sum> md5s_;

    progress_number_callback progress_number_callback_;
    version_read_num_callback version_read_num_callback_;

    std::shared_ptr<ParserDbAdapter> db_adapter_;
    std::shared_ptr<ParserTimeGenerator> time_generator_;

    std::string uuid_;
    std::string title_;
    std::string nickname_;
    std::string source_name_;
    std::string source_url_;
    std::string author_;

    std::string target_url_;
    std::string local_des_;
    std::string parser_name_;

    time_t last_update_date_;
    size_t index_;
    size_t target_start_index_;
    size_t target_end_index_;

    std::mutex mutex_;
    parser_t parser_type_;
    parser_behavior_t parser_behavior_;
    std::atomic_bool done_;
    std::atomic_bool first_curl_wait_done_;

private:
    int FirstCurlWait();
};

size_t HandleCurlResponse(void* prt, size_t size, size_t nmemb, void* data);

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
