SET(SRCS
    steamworks_interface/client.cpp
    steamworks_interface/csgo/csgo_client.cpp
)

SET(HEADERS
    steamworks_interface/client.h
    steamworks_interface/csgo/csgo_client.h
)

add_library(steamworks_interface
    STATIC
    ${SRCS}
    ${HEADERS}
)

target_include_directories(steamworks_interface
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service/steamworks_interface
    ${CMAKE_SOURCE_DIR}/client_service/shared
    ${CMAKE_SOURCE_DIR}/deps/steamworks_sdk/public
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(steamworks_interface
    protobuf::libprotobuf
    steam_protobuf_csgo
)