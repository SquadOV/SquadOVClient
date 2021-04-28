add_executable(csgo_demo_retriever
    src/main.cpp
    src/csgo_demo_retriever.exe.manifest
)

target_include_directories(csgo_demo_retriever
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/client_service/steamworks_interface
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(csgo_demo_retriever
    ${Boost_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/deps/steamworks_sdk/redistributable_bin/win64/steam_api64.lib
    squadov_shared
    steamworks_interface
    date-tz
    dbghelp
)

set_target_properties(csgo_demo_retriever PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/csgo"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/csgo"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/csgo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MinSizeRel/csgo"
)

if(WIN32)
    file(GLOB PREBUILT_DLLS "${CMAKE_SOURCE_DIR}/deps/steamworks_sdk/redistributable_bin/win64/*.dll")
    add_custom_command(
        TARGET csgo_demo_retriever
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PREBUILT_DLLS} $<TARGET_FILE_DIR:csgo_demo_retriever>
        COMMENT "Copying Steamwork DLLs...\n"
    )

    add_custom_command(
        TARGET csgo_demo_retriever
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/client_service/csgo_demo_retriever/steam_appid.txt" $<TARGET_FILE_DIR:csgo_demo_retriever>
        COMMENT "Copying CSGO steam_appid.txt...\n"
    )
endif()