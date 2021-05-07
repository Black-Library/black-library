cmake_minimum_required(VERSION 3.5)

project(BlackLibraryProject VERSION 0.1 LANGUAGES C CXX)

set(BUILD_SHARED_LIBS ON)

set(GLOBAL_BLACKLIBRARYCOMMON ${PROJECT_SOURCE_DIR}/ext)

add_compile_options(-Wall -Wextra -Werror)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(cmake/docker.cmake)

add_subdirectory(ext)
add_subdirectory(src)

add_app_docker_targets()