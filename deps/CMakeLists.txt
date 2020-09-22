add_subdirectory(zlib)
add_subdirectory(tinyxml2)

set (CMAKE_USE_SCHANNEL ON CACHE BOOL "")
add_subdirectory(curl)

set(BUILD_TZ_LIB ON CACHE BOOL "")
add_subdirectory(date)

add_subdirectory(sqlite3)

set(PNG_BUILD_ZLIB ON CACHE BOOL "")
set(SKIP_INSTALL_ALL ON CACHE BOOL "")
add_subdirectory(libpng)


set(PA_ENABLE_DEBUG_OUTPUT OFF)
add_subdirectory(portaudio)