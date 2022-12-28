/**
 * DBConnectionInterfaceUtils.h
 */

#ifndef __BLACK_LIBRARY_CORE_DB_DBCONNECTIONINTERFACEUTILS_H__
#define __BLACK_LIBRARY_CORE_DB_DBCONNECTIONINTERFACEUTILS_H__

#include <DBConnectionInterface.h>

namespace black_library {

namespace core {

namespace db {

std::string GetMediaTypeString(DBEntryMediaType media_type)
{
    switch (media_type)
    {
    case DBEntryMediaType::DBEntryMediaTypeError:
        return "error";
    case DBEntryMediaType::Document:
        return "document";
    case DBEntryMediaType::ImageGallery:
        return "image-gallery";
    case DBEntryMediaType::Video:
        return "video";
    default:
        return "unknown error";
    }
}

std::string GetMediaSubtypeString(DBEntryMediaSubtype subtype)
{
    switch (subtype)
    {
    case DBEntryMediaSubtype::DBEntryMediaSubtypeError:
        return "error";
    case DBEntryMediaSubtype::BLOG:
        return "blog";
    case DBEntryMediaSubtype::BOOK:
        return "book";
    case DBEntryMediaSubtype::MANGA:
        return "manga";
    case DBEntryMediaSubtype::MOVIE:
        return "movie";
    case DBEntryMediaSubtype::NEWS_ARTICLE:
        return "news-article";
    case DBEntryMediaSubtype::PAPER:
        return "paper";
    case DBEntryMediaSubtype::PHOTO_ALBUM:
        return "photo-album";
    case DBEntryMediaSubtype::TV_SHOW:
        return "tv-show";
    case DBEntryMediaSubtype::WEBNOVEL:
        return "webnovel";
    case DBEntryMediaSubtype::YOUTUBE:
        return "youtube";
    default:
        return "unknown error";
    }
}

} // namespace db
} // namespace core
} // namespace black_library

#endif
