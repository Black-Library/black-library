add_subdirectory(binder)

add_library(blacklibrary BlackLibrary.cc WgetUrlPuller.cc)
add_library(blacklibrarycli BlackLibraryCLI.cc)
target_link_libraries(blacklibrary blacklibrarydb blacklibraryparsers ScraperSanatizer)
target_link_libraries(blacklibrarycli blacklibrarybinder blacklibrarydb)
target_include_directories(blacklibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(blacklibrarycli PRIVATE ${PROJECT_SOURCE_DIR}/include)

install(
  TARGETS 

  blacklibrary
  blacklibrarycli

  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(blacklibrary PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")
set_target_properties(blacklibrarycli PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

add_executable(blacklibrary_app blacklibrary.cc)
add_executable(blacklibrary_cli blacklibrary_cli.cc)

target_link_libraries(blacklibrary_app blacklibrary)
target_link_libraries(blacklibrary_cli blacklibrary blacklibrarycli)

set_target_properties(blacklibrary_app PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")
set_target_properties(blacklibrary_cli PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS 
    blacklibrary_app 
    blacklibrary_cli
    RUNTIME DESTINATION bin)

if (BUILD_GUI)
    add_library(blacklibrarygui BlackLibraryGUI.cc)
    target_link_libraries(blacklibrarygui blacklibrarybinder blacklibrarydb dearimguiadapter)
    target_include_directories(blacklibrarygui PRIVATE ${PROJECT_SOURCE_DIR}/include)

    install(
    TARGETS 

    blacklibrarygui

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    set_target_properties(blacklibrarygui PROPERTIES INSTALL_RPATH
        "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

    add_executable(blacklibrary_gui blacklibrary_gui.cc)
    target_link_libraries(blacklibrary_gui blacklibrary blacklibrarygui)
    set_target_properties(blacklibrary_gui PROPERTIES INSTALL_RPATH
        "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")

    install(TARGETS 
        blacklibrary_gui
        RUNTIME DESTINATION bin)
endif()
