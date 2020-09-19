add_subdirectory(shared/valorant/content)

SET(SRCS
    shared/games.cpp
    shared/valorant/valorant.cpp
    shared/time.cpp
    shared/strings/strings.cpp
    shared/filesystem/common_paths.cpp
)

SET(HEADERS
    shared/games.h
    shared/valorant/valorant.h
    shared/riot/riot.h
    shared/constants.h
    shared/time.h
    shared/strings/strings.h
    shared/filesystem/common_paths.h
)

add_library(squadov_shared
    STATIC
    ${SRCS}
    ${HEADERS}
)
add_dependencies(squadov_shared valorant-v1.08)

target_include_directories(squadov_shared
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/deps/date/include
)