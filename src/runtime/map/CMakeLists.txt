add_library(eldoria_runtime_map STATIC
    RegionBuilder.cpp
    RegionProvider.cpp
)

add_library(
    Eldoria::MapRuntime
    ALIAS eldoria_runtime_map
)

target_include_directories(
    eldoria_runtime_map
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(
    eldoria_runtime_map
    PUBLIC
        Eldoria::Assets
        Eldoria::World
)
