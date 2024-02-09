/**
 * ParserNetworkAdapter.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_PARSER_NETWORK_ADAPTER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_PARSER_NETWORK_ADAPTER_H__

#include <string>

#include <curl/curl.h>

namespace black_library {

namespace core {

namespace parsers {

class ParserNetworkAdapter
{
public:
    virtual std::string RequestUrl(const std::string url) = 0;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
