find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

message(STATUS "LibXml2 include dir: ${LIBXML2_INCLUDE_DIR}")

include(GNUInstallDirs)

add_library(blacklibraryparserstestparsers
    TestIndexEntryParser.cc
    )

target_link_libraries(blacklibraryparserstestparsers blacklibraryparsers)
target_include_directories(blacklibraryparserstestparsers PUBLIC ${CURL_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include/parsers ${PROJECT_SOURCE_DIR}/include/parsers/test-parsers)

install(
  TARGETS blacklibraryparserstestparsers
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(blacklibraryparserstestparsers PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")

message(STATUS "linker path: $LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")
