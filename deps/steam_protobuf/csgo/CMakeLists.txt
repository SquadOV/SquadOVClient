SET(PROTO_FILES
    cstrike15_gcmessages.proto
    steammessages.proto
    engine_gcmessages.proto
    gcsystemmsgs.proto
    gcsdk_gcmessages.proto
)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
add_library(steam_protobuf_csgo
    STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(steam_protobuf_csgo INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(steam_protobuf_csgo
    protobuf::libprotobuf
)