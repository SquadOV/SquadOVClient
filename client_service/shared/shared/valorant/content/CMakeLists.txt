add_custom_target(
    valorant-v1.08 ALL
    DEPENDS v1.08/agents.h.in
            v1.08/maps.h.in
            v1.08/equips.h.in
            v1.08/gameModes.h.in
)

add_custom_command(
    OUTPUT v1.08/agents.h.in
           v1.08/maps.h.in
           v1.08/equips.h.in
           v1.08/gameModes.h.in
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/v1.08
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/generate.py --version v1.08 --input ${CMAKE_CURRENT_SOURCE_DIR}/v1.08.json --output ${CMAKE_CURRENT_SOURCE_DIR}/v1.08
    COMMENT "Generating Valorant v1.08 content data."
    DEPENDS v1.08.json CMakeLists.txt
    VERBATIM
)