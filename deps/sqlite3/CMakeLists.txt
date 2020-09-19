add_library(sqlite3 SHARED
    sqlite3.h
    sqlite3.c
)

target_compile_definitions(sqlite3 PUBLIC "-DNO_TCL")