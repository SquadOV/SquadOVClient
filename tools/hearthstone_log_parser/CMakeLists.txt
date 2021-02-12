SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(hearthstone_log_parser
    ${SRCS}
    ${HEADERS}
)

target_include_directories(hearthstone_log_parser
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(hearthstone_log_parser
    squadov_game_event_watcher
    squadov_shared
    ${Boost_LIBRARIES}
    date-tz
)