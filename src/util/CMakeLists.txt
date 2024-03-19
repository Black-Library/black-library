include(GNUInstallDirs)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_library(blacklibrarycommon
    ConfigOperations.cc
    FileOperations.cc
    LogOperations.cc
    JsonOperations.cc
    SourceInformation.cc
    StringOperations.cc
    TimeOperations.cc
    VersionOperations.cc
    )

target_include_directories(blacklibrarycommon PUBLIC ${PROJECT_SOURCE_DIR}/include/util)
target_link_libraries(blacklibrarycommon nlohmann_json::nlohmann_json OpenSSL::SSL stdc++fs spdlog::spdlog)

install(
  TARGETS blacklibrarycommon
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(blacklibrarycommon PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")
