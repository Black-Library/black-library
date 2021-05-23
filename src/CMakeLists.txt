add_library(BlackLibrary BlackLibrary.cc WgetUrlPuller.cc)
add_library(BlackLibraryCLI BlackLibraryCLI.cc)
target_link_libraries(BlackLibrary BlackLibraryDB blacklibraryparsers ScraperSanatizer)
target_link_libraries(BlackLibraryCLI BlackLibraryDB)
target_include_directories(BlackLibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(BlackLibraryCLI PRIVATE ${PROJECT_SOURCE_DIR}/include)

install(
  TARGETS 

  BlackLibrary
  BlackLibraryCLI

  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(BlackLibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")
set_target_properties(BlackLibraryCLI PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

add_executable(blacklibrary blacklibrary.cc)
add_executable(blacklibrary_cli blacklibrary_cli.cc)

target_link_libraries(blacklibrary BlackLibrary)
target_link_libraries(blacklibrary_cli BlackLibrary BlackLibraryCLI)

set_target_properties(blacklibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")
set_target_properties(blacklibrary_cli PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS 
    blacklibrary 
    blacklibrary_cli
    RUNTIME DESTINATION bin)
