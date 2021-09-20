add_subdirectory(black-library-binder)
add_subdirectory(black-library-common)
add_subdirectory(black-library-db)
add_subdirectory(black-library-parsers)
add_subdirectory(scraper-sanatizer)

if (BUILD_GUI)
    message("BUILD_GUI ON")
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

        ${imgui_libs}/gl3w/GL/gl3w.c
    )

    target_include_directories(dearimguiadapter PUBLIC
        ${imgui_root}
        ${imgui_backends}
        ${imgui_libs}/gl3w/
        ${imgui_libs}/glfw/include/
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
    message("BUILD_GUI OFF")
endif()
