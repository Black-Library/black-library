/**
 * CurlOperations.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__
#define __BLACK_LIBRARY_CORE_COMMON_CURL_OPERATIONS_H__

#include <string>

#include <curl/curl.h>

// https://github.com/PwRAu/scraf-backend/blob/9edcd323b8d174aaf6ce29fd7f5a4456c56624f0/tests/students.cpp#L31C1-L52
// https://github.com/Tachi107/scrafurl/blob/main/include/scrafurl.hpp

namespace black_library {

namespace core {

namespace common {

class CurlAdapter {
public:
    CurlAdapter();
    ~CurlAdapter();

    void CurlGet(const std::string &url);
    void CurlPost(const std::string &url, const std::string &request);
    void CurlPut(const std::string &url, const std::string &request);
    void CurlDelete(const std::string &url, const std::string &request);
private:
    CURL* curl_;
};

} // namespace common
} // namespace core
} // namespace black_library

#endif
