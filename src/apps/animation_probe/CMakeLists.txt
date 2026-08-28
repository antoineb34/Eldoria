add_executable(animation_probe
    main.cpp
)

target_include_directories(animation_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_probe PRIVATE
    Eldoria::Data
)

set_target_properties(animation_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


add_executable(animation_data_check
    data_check.cpp
)

target_include_directories(animation_data_check PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_data_check PRIVATE
    Eldoria::Data
)

set_target_properties(animation_data_check PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


add_executable(animation_spotanim_probe
    spotanim_compat.cpp
)

target_include_directories(animation_spotanim_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_spotanim_probe PRIVATE
    Eldoria::Data
)

set_target_properties(animation_spotanim_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


add_executable(animation_visual_probe
    visual_probe.cpp
)

target_include_directories(animation_visual_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_visual_probe PRIVATE
    Eldoria::Data
    Eldoria::Platform
)

set_target_properties(animation_visual_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(animation_spotanim_candidates
    spotanim_candidates.cpp
)

target_include_directories(animation_spotanim_candidates PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_spotanim_candidates PRIVATE
    Eldoria::Data
)

set_target_properties(animation_spotanim_candidates PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


add_executable(animation_npc_probe
    npc_walk_probe.cpp
)

target_include_directories(animation_npc_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(animation_npc_probe PRIVATE
    Eldoria::Data
    Eldoria::Platform
    Eldoria::Graphics
    Eldoria::Render
    Eldoria::Math
)

set_target_properties(animation_npc_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
