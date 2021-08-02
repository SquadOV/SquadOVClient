SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(wow_log_watcher
    ${SRCS}
    ${HEADERS}
)

target_include_directories(wow_log_watcher
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(wow_log_watcher
    squadov_game_event_watcher
    squadov_shared
    ${Boost_LIBRARIES}
    date-tz
    dbghelp
)