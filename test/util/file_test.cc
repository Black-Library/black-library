/**
 * file_test.cc
 */

#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <FileOperations.h>

namespace black_library {

namespace core {

namespace common {

// creates files in the /tmp/ directory
void CreateFile(const std::string &file_name)
{
    std::ofstream ofs("/tmp/" + file_name);
    ofs << "test input";
    ofs.close();
}

TEST_CASE( "CheckFilePermission generic tests (pass)", "[single-file]" )
{
    REQUIRE( CheckFilePermission("/tmp/") == true );
    REQUIRE( CheckFilePermission("/tmp") == true );
    REQUIRE( CheckFilePermission("-this-file-does-not-exist-") == false );
}

TEST_CASE( "PathExists generic tests (pass)", "[single-file]")
{
    CreateFile("test-file.txt");
    REQUIRE( PathExists("/tmp/test-file.txt") == true );
    REQUIRE( PathExists("-this-file-does-not-exist-") == false );
    REQUIRE( RemovePath("/tmp/test-file.txt") == true );
}

TEST_CASE( "PathExistsAndPermission generic tests (pass)", "[single-file]")
{
    REQUIRE( PathExistsAndPermission("/tmp/") == true );
    REQUIRE( PathExistsAndPermission("-this-file-does-not-exist-") == false );
}

TEST_CASE( "MakeDirectories generic tests (pass)", "[single-file]")
{
    REQUIRE( MakeDirectories("/tmp/test-dir") == true );
    REQUIRE( MakeDirectories("/tmp/test-dir") == true );
    REQUIRE( PathExists("/tmp/test-dir/") == true );
    REQUIRE( RemovePath("/tmp/test-dir/") == true );
}

TEST_CASE( "RemovePath generic tests (pass)", "[single-file]")
{
    CreateFile("test-file.txt");
    REQUIRE( RemovePath("/tmp/test-file.txt") == true );
    REQUIRE( RemovePath("/tmp/test-file.txt") == false );
}

TEST_CASE( "SanatizeFileName generic tests (pass)", "[single-file]" )
{
    REQUIRE( SanitizeFileName("") == "" );
    REQUIRE( SanitizeFileName(" /\\*?<>:;=[]!@|.,%#'\"") == "" );
    REQUIRE( SanitizeFileName("SEC0114_segment-\"when-they-say-'\''stand-at-your-side'\''-they'\''re-really-saying-'\''i-love-you'\''\"") == "SEC0114_segment-when-they-say-stand-at-your-side-they-re-really-saying-i-love-you" );
    REQUIRE( SanitizeFileName("Prologue: I'M A MOTHER GOO! NOT A SINGLE MOM!") == "Prologue-I-M-A-MOTHER-GOO-NOT-A-SINGLE-MOM" );
}

TEST_CASE( "SanatizeFilePath generic tests (pass)", "[single-file]" )
{
    REQUIRE( SanitizeFilePath("") == "" );
    REQUIRE( SanitizeFilePath(" *?<>:;=[]!@|") == "-------------" );
    REQUIRE( SanitizeFilePath("- *?<>:;=[]!@|") == "-------------" );
    REQUIRE( SanitizeFilePath("/mnt/black-library/output/") == "/mnt/black-library/output/" );
    REQUIRE( SanitizeFilePath("/mn*t/bla!ck-li:bra?ry/o@ut;pu|t/") == "/mn-t/bla-ck-li-bra-ry/o-ut-pu-t/" );
}

} // namespace common
} // namespace core
} // namespace black_library
