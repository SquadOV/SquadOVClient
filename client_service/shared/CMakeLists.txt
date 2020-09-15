SET(SRCS
    shared/games.cpp
    shared/valorant/valorant.cpp
    shared/time.cpp
)

SET(HEADERS
    shared/games.h
    shared/valorant/valorant.h
    shared/riot/riot.h
    shared/constants.h
    shared/time.h
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
)