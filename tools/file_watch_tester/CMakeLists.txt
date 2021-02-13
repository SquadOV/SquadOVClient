SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(file_watch_tester
    ${SRCS}
    ${HEADERS}
)

target_include_directories(file_watch_tester
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(file_watch_tester
    squadov_game_event_watcher
    squadov_shared
    Boost::program_options
    date-tz
)