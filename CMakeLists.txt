cmake_minimum_required(VERSION 3.15)

# set the project name
project(SquadOVClient VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF" CACHE STRING "" FORCE)

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

find_package(OpenSSL REQUIRED)

set(FFMPEG_ROOT ${CMAKE_SOURCE_DIR}/prebuilt/ffmpeg CACHE STRING "" FORCE)
set(OPENH264_ROOT ${CMAKE_SOURCE_DIR}/prebuilt/openh264 CACHE STRING "" FORCE)
find_package(FFMPEG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)

set(OPENH264_LIBRARIES
    ${OPENH264_ROOT}/lib/openh264.lib
)

# Need to duplicate the work of find_package(ZLIB)
add_library(ZLIB::ZLIB INTERFACE IMPORTED)
list(APPEND _ZLIB_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/zlib")
set_target_properties(ZLIB::ZLIB PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${_ZLIB_INCLUDE_DIRS})
target_link_libraries(ZLIB::ZLIB INTERFACE zlib)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.74.0 REQUIRED
    COMPONENTS filesystem system program_options iostreams zlib)
add_subdirectory(deps)
add_subdirectory(client_service)
add_subdirectory(tools)