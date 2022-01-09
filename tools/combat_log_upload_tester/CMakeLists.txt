SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(combat_log_upload_tester
    ${SRCS}
    ${HEADERS}
)

target_include_directories(combat_log_upload_tester
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(combat_log_upload_tester
    squadov_shared
    ${Boost_LIBRARIES}
    date-tz
    dbghelp
)