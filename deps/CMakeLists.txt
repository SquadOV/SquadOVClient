add_subdirectory(zlib)
add_subdirectory(tinyxml2)

set (CMAKE_USE_SCHANNEL ON CACHE BOOL "")
add_subdirectory(curl)

set(BUILD_TZ_LIB ON CACHE BOOL "")
set(MANUAL_TZ_DB ON CACHE BOOL "")
add_subdirectory(date)

add_subdirectory(sqlite3)

set(PNG_BUILD_ZLIB ON CACHE BOOL "")
set(SKIP_INSTALL_ALL ON CACHE BOOL "")
add_subdirectory(libpng)


set(PA_ENABLE_DEBUG_OUTPUT ON)
add_subdirectory(portaudio)

add_subdirectory(libzmq)

set(CIVETWEB_ENABLE_CXX ON CACHE BOOL "")
add_subdirectory(civetweb)