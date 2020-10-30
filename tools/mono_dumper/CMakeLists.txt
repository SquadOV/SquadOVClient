SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(mono_dumper
    ${SRCS}
    ${HEADERS}
)

target_include_directories(mono_dumper
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/process_watcher
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(mono_dumper
    squadov_process_watcher
    squadov_shared
    Boost::program_options
)