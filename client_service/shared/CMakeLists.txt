SET(SRCS
    shared/games.cpp
    shared/valorant/valorant.cpp
    shared/time.cpp
    shared/strings/strings.cpp
    shared/filesystem/common_paths.cpp
    shared/sqlite/sqlite.cpp
    shared/aimlab/aimlab.cpp
)

SET(HEADERS
    shared/games.h
    shared/valorant/valorant.h
    shared/riot/riot.h
    shared/constants.h
    shared/time.h
    shared/strings/strings.h
    shared/filesystem/common_paths.h
    shared/sqlite/sqlite.h
    shared/aimlab/aimlab.h
)

add_library(squadov_shared
    STATIC
    ${SRCS}
    ${HEADERS}
)

target_include_directories(squadov_shared
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/deps/date/include
    ${CMAKE_SOURCE_DIR}/deps/sqlite3
)