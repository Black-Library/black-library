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
        { "title", entry.title }
    };
}

} // namespace db
} // namespace core
} // namespace black_library
