SET(SRCS
    src/service.cpp
    src/valorant/valorant_process_handler.cpp
    src/valorant/valorant_match.cpp
    src/valorant/valorant_match_details.cpp
    src/valorant/valorant_api.cpp
    src/database/api.cpp
    src/http/http_client.cpp
    src/recorder/game_recorder.cpp
    src/recorder/win32_gdi_recorder.cpp
    src/recorder/video/ffmpeg_video_encoder.cpp
    src/recorder/video/ffmpeg_utils.c
    src/recorder/image/image.cpp
)

SET(HEADERS
    src/valorant/valorant_process_handler.h
    src/valorant/valorant_match.h
    src/valorant/valorant_match_details.h
    src/valorant/valorant_api.h
    src/database/api.h
    src/http/http_client.h
    src/recorder/game_recorder.h
    src/recorder/win32_gdi_recorder.h
    src/recorder/video/ffmpeg_video_encoder.h
    src/recorder/image/image.h
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
    ${CMAKE_SOURCE_DIR}/deps/curl/curl
    ${CMAKE_SOURCE_DIR}/deps/json/single_include
    ${CMAKE_SOURCE_DIR}/deps/sqlite3
    ${CMAKE_SOURCE_DIR}/deps/libpng
    ${OPENSSL_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIR}
)

target_link_libraries(squadov_client_service
    squadov_process_watcher
    squadov_game_event_watcher
    squadov_shared
    tinyxml2
    libcurl
    date-tz
    sqlite3
    png
    ZLIB::ZLIB
    ${OPENSSL_LIBRARIES}
    ${FFMPEG_LIBRARIES}
)

if(WIN32)
    file(GLOB FFMPEG_DLLS "${CMAKE_SOURCE_DIR}/deps/ffmpeg/bin/*.dll")
    add_custom_command(
        TARGET squadov_client_service
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFMPEG_DLLS} $<TARGET_FILE_DIR:squadov_client_service>
    )
endif()