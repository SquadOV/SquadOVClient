add_subdirectory(zlib)
add_subdirectory(tinyxml2)

set (CMAKE_USE_SCHANNEL ON)
add_subdirectory(curl)

set(BUILD_TZ_LIB ON)
add_subdirectory(date)

add_subdirectory(sqlite3)

set(PNG_BUILD_ZLIB ON)
set(SKIP_INSTALL_ALL ON)
add_subdirectory(libpng)