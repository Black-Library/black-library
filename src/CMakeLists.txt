add_library(BlackLibrary BlackLibrary.cc WgetUrlPuller.cc)
target_link_libraries(BlackLibrary BlackLibraryDB ScraperSanatizer)
target_include_directories(BlackLibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(
  TARGETS BlackLibrary
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(BlackLibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

add_executable(blacklibrary blacklibrary.cc)

target_link_libraries(blacklibrary BlackLibrary BlackLibraryDB)

set_target_properties(blacklibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS blacklibrary RUNTIME DESTINATION bin)
