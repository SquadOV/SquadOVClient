SET(SRCS
    main.cpp
)

SET(HEADERS
)

add_executable(shmem_csgo_demo_retriever
    ${SRCS}
    ${HEADERS}
)

target_include_directories(shmem_csgo_demo_retriever
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(shmem_csgo_demo_retriever
    squadov_shared
    ${Boost_LIBRARIES}
    dbghelp
)