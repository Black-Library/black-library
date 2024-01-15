/**
 * SourceInformation.cc
 */

#include <SourceInformation.h>

namespace black_library {

namespace core {

namespace common {

std::string GenerateUrlFromSourceUrl(const char *source_url)
{
    return std::string("https://") + source_url;
}

bool IsSourceInformationMember(const std::string &url)
{
    bool is_member = false;

    if (StartsWithString(url, "#"))
        return is_member;

    if (StartsWithString(url, GenerateUrlFromSourceUrl(AO3::source_url)))
    {
        is_member = true;
    }
    else if (StartsWithString(url, GenerateUrlFromSourceUrl(FFN::source_url)))
    {
        is_member = true;
    }
    else if (StartsWithString(url, GenerateUrlFromSourceUrl(SBF::source_url)))
    {
        is_member = true;
    }
    else if (StartsWithString(url, GenerateUrlFromSourceUrl(SVF::source_url)))
    {
        is_member = true;
    }
    else if (StartsWithString(url, GenerateUrlFromSourceUrl(RR::source_url)))
    {
        is_member = true;
    }
    // Wordpress url exists somewhere in the given url, but not if # in front
    else if ( ContainsString(url, WP::source_url) )
    {
        is_member = true;
    }
    else if (StartsWithString(url, GenerateUrlFromSourceUrl(YT::source_url)))
    {
        is_member = true;
    }

    return is_member;
}

} // namespace common
} // namespace core
} // namespace black_library
