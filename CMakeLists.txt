cmake_minimum_required(VERSION 3.5)

project(BlackLibraryProject VERSION 0.1 LANGUAGES C CXX)

set(BUILD_SHARED_LIBS ON)

set(GLOBAL_BLACKLIBRARYCOMMON ${PROJECT_SOURCE_DIR}/ext)
set(GLOBAL_CATCH2 ${PROJECT_SOURCE_DIR}/ext)

add_compile_options(-Wall -Wextra -Werror)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_GUI "Build the front end GUI" OFF)
option(INCLUDE_DOCKER_BUILD "Add docker targets" ON)
option(REQUIRE_PYTHON "Check for Python Dependencies required for certain adapters and parsers" ON)

if (INCLUDE_DOCKER_BUILD)
    include(cmake/docker.cmake)
endif()

add_subdirectory(ext)
add_subdirectory(src)

add_app_docker_targets()