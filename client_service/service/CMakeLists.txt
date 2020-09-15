SET(SRCS
    src/service.cpp
    src/valorant/valorant_process_handler.cpp
)

SET(HEADERS
    src/valorant/valorant_process_handler.h
)

add_executable(squadov_client_service
    ${SRCS}
    ${HEADERS}
)

target_include_directories(squadov_client_service
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/process_watcher
    ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
    ${CMAKE_SOURCE_DIR}/client_service/service/src
)

target_link_libraries(squadov_client_service
    squadov_process_watcher
    squadov_game_event_watcher
    squadov_shared
    tinyxml2
    libcurl
    date-tz
)