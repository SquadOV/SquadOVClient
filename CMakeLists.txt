cmake_minimum_required(VERSION 3.10)

# set the project name
project(SquadOVClient VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

find_package(OpenSSL REQUIRED)

set(FFMPEG_ROOT ${CMAKE_SOURCE_DIR}/deps/ffmpeg)
find_package(FFMPEG REQUIRED)

# Need to duplicate the work of find_package(ZLIB)
add_library(ZLIB::ZLIB INTERFACE IMPORTED)
list(APPEND _ZLIB_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/zlib")
set_target_properties(ZLIB::ZLIB PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${_ZLIB_INCLUDE_DIRS})
target_link_libraries(ZLIB::ZLIB INTERFACE zlib)

find_package(BOOST 1.74.0 REQUIRED
    COMPONENTS filesystem system lockfree)

add_subdirectory(deps)
add_subdirectory(client_service)