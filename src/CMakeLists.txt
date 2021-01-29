add_executable(blacklibrary blacklibrary.cc)

target_link_libraries(blacklibrary BlackLibraryDB)

set_target_properties(blacklibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS blacklibrary RUNTIME DESTINATION bin)
