SET(SRCS
    process_watcher/watcher.cpp
    process_watcher/process/process.cpp
    process_watcher/games/game_process_detector.cpp
    process_watcher/games/valorant_process_detector.cpp
)

SET(HEADERS
    process_watcher/watcher.h
    process_watcher/handler.h
    process_watcher/process/process.h
    process_watcher/games/game_process_detector.h
    process_watcher/games/valorant_process_detector.h
)

add_library(squadov_process_watcher
    STATIC
    ${SRCS}
    ${HEADERS}
)

target_include_directories(squadov_process_watcher
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/process_watcher
)