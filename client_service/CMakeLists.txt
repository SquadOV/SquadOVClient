add_subdirectory(libinputhooks)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_subdirectory(shared)
    add_subdirectory(game_event_watcher)
    add_subdirectory(process_watcher)
    add_subdirectory(service)
    add_subdirectory(advanced_crash_dump_enabler)
endif()