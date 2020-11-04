SET(SRCS
    game_event_watcher/aimlab/aimlab_log_watcher.cpp
    game_event_watcher/valorant/valorant_log_watcher.cpp
    game_event_watcher/hearthstone/hearthstone_log_watcher.cpp
    game_event_watcher/hearthstone/hearthstone_log_config.cpp
    game_event_watcher/logs/log_watcher.cpp
)

SET(HEADERS
    game_event_watcher/aimlab/aimlab_log_watcher.h
    game_event_watcher/valorant/valorant_log_watcher.h
    game_event_watcher/hearthstone/hearthstone_log_watcher.h
    game_event_watcher/hearthstone/hearthstone_log_config.h
    game_event_watcher/logs/log_watcher.h
)

add_library(squadov_game_event_watcher
    STATIC
    ${SRCS}
    ${HEADERS}
)

target_include_directories(squadov_game_event_watcher
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
    ${CMAKE_SOURCE_DIR}/deps/tinyxml2
    ${CMAKE_SOURCE_DIR}/deps/tomlplusplus/include
)

target_link_libraries(squadov_game_event_watcher
    squadov_shared
    tinyxml2
)