SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(pe_dumper
    ${SRCS}
    ${HEADERS}
)

target_include_directories(pe_dumper
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/process_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(pe_dumper
    squadov_process_watcher
    squadov_shared
    Boost::program_options
    dbghelp
)