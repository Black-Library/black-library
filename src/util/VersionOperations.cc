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

size_t RRIdNumExtract(const std::string &url, version_extract_t extract_type)
{
    // royal road pattern is https://www.royalroad.com/fiction/12345/some-fiction/chapter/1234567/chapter-name
    
    if (extract_type == black_library::core::common::version_extract_t::CHAPTER_SEQ_NUM)
    {
        size_t chapter_find = url.rfind(RR_CHAPTER_IDENTIFIER) + 9;
        size_t slash_find = url.rfind("/");
        return std::stoul(url.substr(chapter_find, slash_find - chapter_find));
    }
    else if (extract_type == black_library::core::common::version_extract_t::WORK_NUM)
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
            return std::stoul(tokens[4]);
        }

        return 0;
    }
    else
    {
        return 0;
    }
}

std::string RRSecIdExtract(const std::string &url)
{
    // royal road pattern is https://www.royalroad.com/fiction/12345/some-fiction/chapter/1234567/chapter-name

    size_t slash_find = url.rfind("/") + 1;

    return url.substr(slash_find);
}

std::string XFSecIdExtract(const std::string &url)
{
    // xen foro pattern is https://forums.spacebattles.com/threads/some-fiction-name.1234567/page-1#post-12345678
    // sometimes without the page-1
    size_t slash_find = url.rfind("/") + 1;
    size_t hash_find = url.rfind("#");

    std::string sec_id = url.substr(slash_find, hash_find - slash_find);

    // if empty return default xf sec id
    if (sec_id.empty())
        return "page-1";

    return sec_id;
}

size_t XFIdNumExtract(const std::string &url, version_extract_t extract_type)
{
    // xen foro pattern is https://forums.spacebattles.com/threads/some-fiction-name.1234567/page-1#post-12345678
    if (extract_type == black_library::core::common::version_extract_t::CHAPTER_SEQ_NUM)
    {
        size_t dash_find = url.rfind("-");
        return std::stoul(url.substr(dash_find + 1));
    }
    else if (extract_type == black_library::core::common::version_extract_t::WORK_NUM)
    {
        size_t period_find = url.rfind(".") + 1;
        size_t slash_find = url.rfind("/");
        return std::stoul(url.substr(period_find, slash_find - period_find));
    }
    else
    {
        return 0;
    }
}

std::string GetMD5Hash(const std::string &input)
{
    return CalculateMd5Manual((unsigned char*)input.c_str(), input.size());
}

std::string GetWorkChapterSecIdFromUrl(const std::string &url)
{
    std::string sec_id = "unknown-source-sec-id";

    if (ContainsString(url, SBF::source_url))
    {
        return XFSecIdExtract(url);
    }
    else if (ContainsString(url, SVF::source_url))
    {
        return XFSecIdExtract(url);
    }
    else if (ContainsString(url, RR::source_url))
    {
        return RRSecIdExtract(url);
    }
    else
    {
        return sec_id;
    }
}

size_t GetWorkChapterSeqNumFromUrl(const std::string &url)
{
    size_t seq_num = MaxSeqNum;

    if (ContainsString(url, SBF::source_url))
    {
        return XFIdNumExtract(url, black_library::core::common::version_extract_t::CHAPTER_SEQ_NUM);
    }
    else if (ContainsString(url, SVF::source_url))
    {
        return XFIdNumExtract(url, black_library::core::common::version_extract_t::CHAPTER_SEQ_NUM);
    }
    else if (ContainsString(url, RR::source_url))
    {
        return RRIdNumExtract(url, black_library::core::common::version_extract_t::CHAPTER_SEQ_NUM);
    }
    else
    {
        return seq_num;
    }
}

size_t GetWorkNumFromUrl(const std::string &url)
{
    size_t work_num = 0;

    if (ContainsString(url, SBF::source_url))
    {
        return XFIdNumExtract(url, black_library::core::common::version_extract_t::WORK_NUM);
    }
    else if (ContainsString(url, SVF::source_url))
    {
        return XFIdNumExtract(url, black_library::core::common::version_extract_t::WORK_NUM);
    }
    else if (ContainsString(url, RR::source_url))
    {
        return RRIdNumExtract(url, black_library::core::common::version_extract_t::WORK_NUM);
    }
    else
    {
        return work_num;
    }
}



} // namespace common
} // namespace core
} // namespace black_library