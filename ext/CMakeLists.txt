# catch2
add_subdirectory(catch2)
install(
    TARGETS
        Catch2
        Catch2WithMain
    EXPORT
        Catch2Targets
    LIBRARY DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_BINDIR}
)

# cpp-httplib
# add_subdirectory(cpp-httplib)

#imgui
message(STATUS "BUILD_GUI: ${BUILD_GUI}")

if (BUILD_GUI)
    message(STATUS "Building GUI")
    include(GNUInstallDirs)

    # set root to be expected imgui location
    set(imgui_root ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    set(imgui_backends ${imgui_root}/backends)
    set(imgui_libs ${imgui_root}/examples/libs)

    add_library(dearimguiadapter
        ${imgui_root}/imgui.cpp
        ${imgui_root}/imgui_demo.cpp
        ${imgui_root}/imgui_draw.cpp
        ${imgui_root}/imgui_tables.cpp
        ${imgui_root}/imgui_widgets.cpp

        ${imgui_backends}/imgui_impl_glfw.cpp
        ${imgui_backends}/imgui_impl_opengl3.cpp

        ${imgui_root}/examples/example_glfw_opengl3/main.cpp
    )

    target_include_directories(dearimguiadapter PUBLIC
        ${imgui_root}
        ${imgui_backends}
        ${imgui_libs}/glfw/include/GLFW
    )
    target_link_libraries(dearimguiadapter GL glfw)

    install(
    TARGETS dearimguiadapter 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    set_target_properties(dearimguiadapter PROPERTIES INSTALL_RPATH
        "$LD_LIBRARY_PATH;${CMAKE_INSTALL_PREFIX}/lib")
else()
    message(STATUS "BUILD_GUI OFF")
endif()

# nlohmann json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

# pistache
set(PISTACHE_BUILD_TESTS OFF CACHE INTERNAL "Skip pistache tests")
message(STATUS "PISTACHE_BUILD_TESTS: ${PISTACHE_BUILD_TESTS}")
add_subdirectory(pistache)

# scraper-sanatizer
add_subdirectory(scraper-sanatizer)

# spdlog
set(SPDLOG_BUILD_SHARED ON CACHE INTERNAL "Build spdlog shared library")
set(SPDLOG_INSTALL ON CACHE INTERNAL "Install spdlog")
message(STATUS "SPDLOG_BUILD_SHARED: ${SPDLOG_BUILD_SHARED}")
message(STATUS "SPDLOG_INSTALL: ${SPDLOG_INSTALL}")
add_subdirectory(spdlog)
