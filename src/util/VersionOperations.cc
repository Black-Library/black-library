/**
 * VersionOperations.cc
 */

#include <iomanip>
#include <sstream>

#include <openssl/evp.h>

#include <SourceInformation.h>
#include <VersionOperations.h>

namespace black_library {

namespace core {

namespace common {

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

std::string GetIdentifierFromUrl(const std::string &url)
{
    return "";
}

std::string GetMD5Hash(const std::string &input)
{
    return CalculateMd5Manual((unsigned char*)input.c_str(), input.size());
}

} // namespace common
} // namespace core
} // namespace black_library