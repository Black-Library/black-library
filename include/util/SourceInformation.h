/**
 * SourceInformation.h
 */

#ifndef __BLACK_LIBRARY_CORE_COMMON_SOURCE_INFORMATION_H__
#define __BLACK_LIBRARY_CORE_COMMON_SOURCE_INFORMATION_H__

#include <string>

#include <StringOperations.h>

namespace black_library {

namespace core {

namespace common {

namespace ERROR {
    static constexpr char full_name[] = "Full Name Error";
    static constexpr char short_name[] = "ERROR";
    static constexpr char source_name[] = "source-name-error";
    static constexpr char source_url[] = "error-source-url";
} // namespace AO3

namespace AO3 {
    static constexpr char full_name[] = "Archive Of Our Own";
    static constexpr char short_name[] = "AO3";
    static constexpr char source_name[] = "archive-of-our-own";
    static constexpr char source_url[] = "archiveofourown.org";
} // namespace AO3

namespace FFN {
    static constexpr char name[] = "FanFiction";
    static constexpr char short_name[] = "FFN";
    static constexpr char source_name[] = "fanfiction";
    static constexpr char source_url[] = "www.fanfiction.net";
} // namespace FFN

namespace RR {
    static constexpr char name[] = "Royal Road";
    static constexpr char short_name[] = "RR";
    static constexpr char source_name[] = "royal-road";
    static constexpr char source_url[] = "www.royalroad.com";
} // namespace RR

namespace SBF {
    static constexpr char name[] = "SpaceBattles Forums";
    static constexpr char short_name[] = "SBF";
    static constexpr char source_name[] = "spacebattles-forums";
    static constexpr char source_url[] = "forums.spacebattles.com";
} // namespace SBF

namespace SVF {
    static constexpr char name[] = "Sufficient Velocity Forums";
    static constexpr char short_name[] = "SVF";
    static constexpr char source_name[] = "sufficient-velocity-forums";
    static constexpr char source_url[] = "forums.sufficientvelocity.com";
} // namespace SBF

namespace WP {
    static constexpr char name[] = "WordPress";
    static constexpr char short_name[] = "WP";
    static constexpr char source_name[] = "wordpress";
    static constexpr char source_url[] = "wordpress.com";
} // namespace WP

namespace YT {
    static constexpr char name[] = "YouTube";
    static constexpr char short_name[] = "YT";
    static constexpr char source_name[] = "youtube";
    static constexpr char source_url[] = "youtube.com";
} // namespace YT

std::string GenerateUrlFromSourceUrl(const char *source_url);
bool IsSourceInformationMember(const std::string &url);

} // namespace common
} // namespace core
} // namespace black_library

#endif
