/**
 * ParserFactory.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_FACTORY_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_FACTORY_H__

#include <memory>
#include <unordered_map>

#include <ConfigOperations.h>

#include "Parser.h"
#include "ParserDbAdapter.h"

namespace black_library {

namespace core {

namespace parsers {

struct ParserFactoryResult {
    std::shared_ptr<Parser> parser_result;
    bool has_error = false;
};

typedef std::function<ParserFactoryResult(void)> factory_function;

class ParserFactory
{
public:
    ParserFactory(const njson &config, const std::shared_ptr<ParserDbAdapter> &db_adapter);
    ~ParserFactory();

    ParserFactoryResult GetParserByUrl(const std::string &url);
    ParserFactoryResult GetParserByType(parser_t parser_type);

protected:
    int InitParserMap(const njson &config);

    std::unordered_map<parser_t, factory_function> parser_map_;
    std::shared_ptr<ParserDbAdapter> db_adapter_;

private:
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
