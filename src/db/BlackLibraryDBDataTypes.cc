/**
 * BlackLibraryDBDataTypes.cc
 */

#include <BlackLibraryDBDataTypes.h>

namespace black_library {

namespace core {

namespace db {

void to_json(njson &j, const DBEntry &entry)
{
    j = njson{
        { "UUID", entry.uuid },
        { "title", entry.title },
        { "author", entry.title },
        { "nickname", entry.nickname },
        { "source", entry.source },
        { "url", entry.url },
        { "last_url", entry.last_url },
        { "series", entry.series },
        { "series_length", entry.series_length },
        { "version", entry.version },
        { "media_path", entry.media_path },
        { "birth_date", entry.birth_date },
        { "check_date", entry.check_date },
        { "update_date", entry.update_date },
        { "user_contributed", entry.user_contributed },
        { "processing", entry.processing },
    };
}

} // namespace db
} // namespace core
} // namespace black_library
