add_executable(advanced_crash_dump_enabler
    src/main.cpp
    src/advanced_crash_dump_enabler.exe.manifest
)

target_link_libraries(advanced_crash_dump_enabler
    Advapi32
)

SET_TARGET_PROPERTIES(advanced_crash_dump_enabler PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
