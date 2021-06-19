SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(hearthstone_dumper
    ${SRCS}
    ${HEADERS}
)

target_include_directories(hearthstone_dumper
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/process_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(hearthstone_dumper
    squadov_process_watcher
    squadov_shared
    ${Boost_LIBRARIES}
    dbghelp
)