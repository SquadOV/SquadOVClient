add_library(
    libinputhooks SHARED
    inputhooks.h
    inputhooks.cpp
)

target_include_directories(libinputhooks
    PUBLIC
    ${CMAKE_SOURCE_DIR}/client_service
    ${CMAKE_SOURCE_DIR}/client_service/shared
)

target_compile_definitions(libinputhooks PRIVATE COMPILING_LIBRARY)