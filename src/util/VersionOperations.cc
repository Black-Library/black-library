/**
 * VersionOperations.cc
 */

#include <iomanip>
#include <iostream>
#include <sstream>

#include <openssl/evp.h>

#include <SourceInformation.h>
#include <StringOperations.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace common {

enum class version_extract_t
{
    CHAPTER,
    WORK,

    _NUM_VERSION_EXTRACT_TYPE
};

static constexpr const char RR_CHAPTER_IDENTIFIER[] = "/chapter/";

std::string CalculateMd5Manual(unsigned char* buf, unsigned int buf_size)
{
    std::ostringstream oss;
    EVP_MD_CTX *mdctx;
    unsigned char *md5_digest;
    unsigned int md5_digest_len = EVP_MD_size(EVP_md5());
    
    // MD5_Init
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    // MD5_Update
    EVP_DigestUpdate(mdctx, buf, buf_size);

    // MD5_Final
    md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_len);
    EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);
    EVP_MD_CTX_free(mdctx);

    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < md5_digest_len; ++i)
    {
        oss << std::setw(2) << (int)md5_digest[i];
    }

    OPENSSL_free(md5_digest);

    return oss.str();
}

std::string RRIdentifierExtract(const std::string &url, version_extract_t extract_type)
{
    // royal road pattern is https://www.royalroad.com/fiction/12345/some-fiction/chapter/1234567/chapter-name
    
    if (extract_type == black_library::core::common::version_extract_t::CHAPTER)
    {
        size_t chapter_find = url.rfind(RR_CHAPTER_IDENTIFIER) + 9;
        size_t slash_find = url.rfind("/");
        return url.substr(chapter_find, slash_find - chapter_find);
    }
    else if (extract_type == black_library::core::common::version_extract_t::WORK)
    {
        std::vector<std::string> tokens;
        std::stringstream ss;
        std::string intermediate;

        ss << url;

        while (getline(ss, intermediate, '/'))
        {
            tokens.push_back(intermediate);
        }

        if (tokens.size() >= 4)
        {
            return tokens[4];
        }

        return "";
    }
    else
    {
        return "";
    }
}

std::string XFIdentifierExtract(const std::string &url, version_extract_t extract_type)
{
    // xen foro pattern is https://forums.spacebattles.com/threads/some-fiction-name.1234567/#post-12345678
    if (extract_type == black_library::core::common::version_extract_t::CHAPTER)
    {
        size_t dash_find = url.rfind("-");
        return url.substr(dash_find + 1);
    }
    else if (extract_type == black_library::core::common::version_extract_t::WORK)
    {
        size_t period_find = url.rfind(".") + 1;
        size_t slash_find = url.rfind("/");
        return url.substr(period_find, slash_find - period_find);
    }
    else
    {
        return "";
    }
}

std::string GetMD5Hash(const std::string &input)
{
    return CalculateMd5Manual((unsigned char*)input.c_str(), input.size());
}

std::string GetWorkChapterIdentifierFromUrl(const std::string &url)
{
    std::string identifier = "";


    if (ContainsString(url, SBF::source_url))
    {
        return XFIdentifierExtract(url, black_library::core::common::version_extract_t::CHAPTER);
    }
    else if (ContainsString(url, SVF::source_url))
    {
        return XFIdentifierExtract(url, black_library::core::common::version_extract_t::CHAPTER);
    }
    else if (ContainsString(url, RR::source_url))
    {
        return RRIdentifierExtract(url, black_library::core::common::version_extract_t::CHAPTER);
    }
    else
    {
        return "";
    }
}

std::string GetWorkIdentifierFromUrl(const std::string &url)
{
    std::string identifier = "";


    if (ContainsString(url, SBF::source_url))
    {
        return XFIdentifierExtract(url, black_library::core::common::version_extract_t::WORK);
    }
    else if (ContainsString(url, SVF::source_url))
    {
        return XFIdentifierExtract(url, black_library::core::common::version_extract_t::WORK);
    }
    else if (ContainsString(url, RR::source_url))
    {
        return RRIdentifierExtract(url, black_library::core::common::version_extract_t::WORK);
    }
    else
    {
        return "";
    }
}



} // namespace common
} // namespace core
} // namespace black_library