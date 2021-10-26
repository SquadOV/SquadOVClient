add_custom_target(squadov_build_tests)

MACRO(GENERATE_TEST TEST_NAME CPP_NAME)
    add_executable(${TEST_NAME} ${CPP_NAME})

    target_include_directories(${TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/client_service/shared
        ${CMAKE_SOURCE_DIR}/client_service/process_watcher
        ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
        ${Boost_INCLUDE_DIRS}
    )

    target_link_libraries(${TEST_NAME}
        squadov_shared
        squadov_game_event_watcher
        ${Boost_LIBRARIES}
        date-tz
        dbghelp
    )

    add_test(NAME ${TEST_NAME}_TEST COMMAND ${TEST_NAME})
    add_dependencies(squadov_build_tests ${TEST_NAME})

    target_compile_definitions(${TEST_NAME} PUBLIC "SQUADOV_TZDATA=${CMAKE_SOURCE_DIR}/resources/tzdata")
ENDMACRO()

add_subdirectory(shared)
add_subdirectory(process_watcher)
add_subdirectory(game_event_watcher)