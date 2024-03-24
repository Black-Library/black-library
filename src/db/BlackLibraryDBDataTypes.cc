/**
 * BlackLibraryDBDataTypes.cc
 */

#include <BlackLibraryDBDataTypes.h>

namespace black_library {

namespace core {

namespace db {

void from_json(const njson& j, DBEntry &entry)
{
    j.at("uuid").get_to(entry.uuid);
    j.at("title").get_to(entry.title);
    j.at("author").get_to(entry.author);
    j.at("nickname").get_to(entry.nickname);
    j.at("source").get_to(entry.source);
    j.at("url").get_to(entry.url);
    j.at("last_url").get_to(entry.last_url);
    j.at("series").get_to(entry.series);
    j.at("series_length").get_to(entry.series_length);
    j.at("version").get_to(entry.version);
    j.at("media_path").get_to(entry.media_path);
    j.at("birth_date").get_to(entry.birth_date);
    j.at("check_date").get_to(entry.check_date);
    j.at("update_date").get_to(entry.update_date);
    j.at("user_contributed").get_to(entry.user_contributed);
    j.at("processing").get_to(entry.processing);
}

void to_json(njson &j, const DBEntry &entry)
{
    j = njson{
        { "UUID", entry.uuid },
        { "title", entry.title },
        { "author", entry.author },
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

void to_json(njson &j, const DBErrorEntry &entry)
{
    j = njson{
        { "UUID", entry.uuid },
        { "progress_num", entry.progress_num },
    };
}

} // namespace db
} // namespace core
} // namespace black_library
