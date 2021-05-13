SET(SRCS
    src/api/kafka_api.cpp
    src/api/squadov_api.cpp
    src/api/league_ingame_api.cpp
    src/aimlab/aimlab_db_interface.cpp
    src/aimlab/aimlab_process_handler.cpp
    src/csgo/csgo_process_handler.cpp
    src/valorant/valorant_process_handler.cpp
    src/valorant/valorant_match.cpp
    src/hearthstone/hearthstone_process_handler.cpp
    src/wow/wow_process_handler.cpp
    src/league/league_process_handler.cpp
    src/recorder/game_recorder.cpp
    src/recorder/video/win32_gdi_recorder.cpp
    src/recorder/video/dxgi_desktop_recorder.cpp
    src/recorder/video/windows_graphics_capture.cpp
    src/recorder/encoder/ffmpeg_av_encoder.cpp
    src/recorder/encoder/ffmpeg_utils.c
    src/recorder/encoder/ffmpeg_video_swap_chain.cpp
    src/recorder/image/image.cpp
    src/recorder/image/d3d_image.cpp
    src/recorder/audio/portaudio_audio_recorder.cpp
    src/recorder/audio/device.cpp
    src/recorder/pipe/pipe.cpp
    src/recorder/pipe/file_output_piper.cpp
    src/recorder/pipe/filesystem_piper.cpp
    src/recorder/pipe/gcs_piper.cpp
    src/system/win32/hwnd_utils.cpp
    src/system/win32/message_loop.cpp
    src/system/settings.cpp
    src/system/state.cpp
    src/zeromq/zeromq.cpp
    src/local/local_data.cpp
    src/vod/vod_clipper.cpp
    src/vod/process.cpp
    src/renderer/d3d11_context.cpp
    src/renderer/d3d11_model.cpp
    src/renderer/d3d11_renderer.cpp
    src/renderer/d3d11_shader.cpp
    src/renderer/d3d11_video_processor.cpp
    src/renderer/d3d11_texture.cpp
)

SET(HEADERS
    src/api/squadov_api.h
    src/api/kafka_api.h
    src/api/league_ingame_api.h
    src/aimlab/aimlab_db_interface.h
    src/aimlab/aimlab_process_handler.h
    src/csgo/csgo_process_handler.h
    src/valorant/valorant_process_handler.h
    src/valorant/valorant_match.h
    src/hearthstone/hearthstone_process_handler.h
    src/wow/wow_process_handler.h
    src/league/league_process_handler.h
    src/recorder/game_recorder.h
    src/recorder/video/video_recorder.h
    src/recorder/video/win32_gdi_recorder.h
    src/recorder/video/dxgi_desktop_recorder.h
    src/recorder/video/windows_graphics_capture.h
    src/recorder/encoder/ffmpeg_av_encoder.h
    src/recorder/encoder/ffmpeg_utils.h
    src/recorder/encoder/av_encoder.h
    src/recorder/encoder/ffmpeg_video_swap_chain.h
    src/recorder/image/image.h
    src/recorder/image/d3d_image.h
    src/recorder/audio/audio_recorder.h
    src/recorder/audio/audio_packet_view.h
    src/recorder/audio/audio_packet_properties.h
    src/recorder/audio/fixed_size_audio_packet.h
    src/recorder/audio/portaudio_audio_recorder.h
    src/recorder/audio/device.h
    src/recorder/pipe/pipe.h
    src/recorder/pipe/file_output_piper.h
    src/recorder/pipe/filesystem_piper.h
    src/recorder/pipe/gcs_piper.h
    src/system/win32/hwnd_utils.h
    src/system/win32/message_loop.h
    src/system/settings.h
    src/system/state.h
    src/system/ipc.h
    src/zeromq/zeromq.h
    src/local/local_data.h
    src/vod/vod_clipper.h
    src/vod/process.h
    src/renderer/d3d11_context.h
    src/renderer/d3d11_model.h
    src/renderer/d3d11_renderer.h
    src/renderer/d3d11_shader.h
    src/renderer/d3d11_video_processor.h
    src/renderer/d3d11_texture.h
)

MACRO(CREATE_SERVICE_EXECUTABLE EXE_NAME CPP_NAME MANIFEST_FILE)
    add_executable(${EXE_NAME}
        ${CPP_NAME}
        ${MANIFEST_FILE}
        ${SRCS}
        ${HEADERS}
    )

    target_include_directories(${EXE_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/client_service/shared
        ${CMAKE_SOURCE_DIR}/client_service/process_watcher
        ${CMAKE_SOURCE_DIR}/client_service/game_event_watcher
        ${CMAKE_SOURCE_DIR}/client_service/service/src
        ${CMAKE_SOURCE_DIR}/deps/curl/curl
        ${CMAKE_SOURCE_DIR}/deps/json/single_include
        ${CMAKE_SOURCE_DIR}/deps/sqlite3
        ${CMAKE_SOURCE_DIR}/deps/libpng
        ${CMAKE_SOURCE_DIR}/deps/portaudio/include
        ${CMAKE_SOURCE_DIR}/deps/cppzmq
        ${CMAKE_SOURCE_DIR}/deps/libzmq/include
        ${OPENSSL_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
    )

    target_link_libraries(${EXE_NAME}
        squadov_process_watcher
        squadov_game_event_watcher
        squadov_shared
        tinyxml2
        libcurl
        date-tz
        sqlite3
        png
        portaudio
        libzmq
        civetweb-cpp
        ZLIB::ZLIB
        RdKafka::rdkafka RdKafka::rdkafka++
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${OPENH264_LIBRARIES}
    )

    if(WIN32)
        add_dependencies(${EXE_NAME} libwgc)

        file(GLOB PREBUILT_DLLS "${CMAKE_SOURCE_DIR}/prebuilt/*/bin/*.dll")
        add_custom_command(
            TARGET ${EXE_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PREBUILT_DLLS} $<TARGET_FILE_DIR:${EXE_NAME}>
            COMMENT "Copying DLLs...\n"
        )

        SET(PREBUILT_EXES "${CMAKE_SOURCE_DIR}/prebuilt/ffmpeg/bin/ffmpeg.exe")
        add_custom_command(
            TARGET ${EXE_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PREBUILT_EXES} $<TARGET_FILE_DIR:${EXE_NAME}>
            COMMENT "Copying EXEs...\n"
        )

        file(GLOB HLSL_SHADERS "${CMAKE_SOURCE_DIR}/client_service/service/src/renderer/shaders/*.fxc")
        add_custom_command(
            TARGET ${EXE_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${EXE_NAME}>/shaders
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${HLSL_SHADERS} $<TARGET_FILE_DIR:${EXE_NAME}>/shaders
            COMMENT "Copying Shaders...\n"
        )

        target_link_libraries(${EXE_NAME}
            d3d11
            dxgi
            dbghelp
            Advapi32
        )
    endif()

ENDMACRO()

if(WIN32)
    add_library(
        libwgc SHARED
        libwgc/wgc.h
        libwgc/wgc.cpp
        src/recorder/image/image.cpp
        src/recorder/image/image.h
        src/recorder/image/d3d_image.cpp
        src/recorder/image/d3d_image.h
        src/renderer/d3d11_context.h
        src/renderer/d3d11_context.cpp
        src/renderer/d3d11_texture.h
        src/renderer/d3d11_texture.cpp
        src/renderer/d3d11_renderer.h
        src/renderer/d3d11_renderer.cpp
        src/renderer/d3d11_model.h
        src/renderer/d3d11_model.cpp
        src/renderer/d3d11_shader.h
        src/renderer/d3d11_shader.cpp
    )

    target_include_directories(libwgc
        PUBLIC
        ${CMAKE_SOURCE_DIR}/client_service/service
        ${CMAKE_SOURCE_DIR}/client_service/service/src
        ${CMAKE_SOURCE_DIR}/client_service/shared
        ${CMAKE_SOURCE_DIR}/deps/date/include
        ${CMAKE_SOURCE_DIR}/deps/json/single_include
        ${CMAKE_SOURCE_DIR}/deps/libpng
    )

    target_link_libraries(libwgc
        squadov_shared
        date-tz
        png
        dbghelp
        ${Boost_LIBRARIES}
    )
endif()

CREATE_SERVICE_EXECUTABLE(squadov_client_service "src/service.cpp" "src/squadov_client_service.exe.manifest")
CREATE_SERVICE_EXECUTABLE(wow_finder "src/wow_finder.cpp" "src/wow_finder.exe.manifest")
CREATE_SERVICE_EXECUTABLE(vod_record_tester "src/vod_record_tester.cpp" "src/vod_record_tester.exe.manifest")
CREATE_SERVICE_EXECUTABLE(manual_clipper "src/manual_clipper.cpp" "src/manual_clipper.exe.manifest")
CREATE_SERVICE_EXECUTABLE(d3d11_image_tester "src/d3d11_image_tester.cpp" "src/d3d11_image_tester.exe.manifest")