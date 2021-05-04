add_custom_target(squadov_build_tests)

MACRO(GENERATE_TEST TEST_NAME CPP_NAME)
    add_executable(${TEST_NAME} ${CPP_NAME})

    target_include_directories(${TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/client_service/shared
        ${Boost_INCLUDE_DIRS}
    )

    target_link_libraries(${TEST_NAME}
        squadov_shared
        ${Boost_LIBRARIES}
    )

    add_test(NAME ${TEST_NAME}_TEST COMMAND ${TEST_NAME})
    add_dependencies(squadov_build_tests ${TEST_NAME})
ENDMACRO()

add_subdirectory(shared)