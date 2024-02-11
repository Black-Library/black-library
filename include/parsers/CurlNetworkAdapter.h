/**
 * CurlNetworkAdapter.h
 */

#ifndef __BLACK_LIBRARY_CORE_PARSERS_CURL_NETWORK_ADAPTER_H__
#define __BLACK_LIBRARY_CORE_PARSERS_CURL_NETWORK_ADAPTER_H__

#include "ParserNetworkAdapter.h"

namespace black_library {

namespace core {

namespace parsers {

class CurlNetworkAdapter : public ParserNetworkAdapter
{
public:
    CurlNetworkAdapter(const std::string &logger_name);

    std::string RequestUrl(const std::string url) const override;

private:
    std::string logger_name_;
};

} // namespace parsers
} // namespace core
} // namespace black_library

#endif
