include(GNUInstallDirs)

add_library(blacklibrarybinder
    BlackLibraryBinder.cc
    )

target_link_libraries(blacklibrarybinder blacklibrarycommon)
target_include_directories(blacklibrarybinder PUBLIC ${PROJECT_SOURCE_DIR}/include/binder)

install(
    TARGETS blacklibrarybinder
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(blacklibrarybinder PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")

add_executable(binder_driver binder_driver.cc)

target_link_libraries(binder_driver blacklibrarybinder)

set_target_properties(binder_driver PROPERTIES INSTALL_RPATH
    "$LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS binder_driver RUNTIME DESTINATION bin)
