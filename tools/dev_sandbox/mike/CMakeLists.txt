SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(mike_dev_sandbox
    ${SRCS}
    ${HEADERS}
)

target_include_directories(mike_dev_sandbox
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(mike_dev_sandbox
    squadov_shared
    ${Boost_LIBRARIES}
    date-tz
    dbghelp
)