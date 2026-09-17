add_library(eldoria_world STATIC
    Terrain.cpp
    Pathfinder.cpp
    World.cpp
)

add_library(
    Eldoria::World
    ALIAS eldoria_world
)

# Expose runtime/ so consumers can write:
#
#   #include "world/Terrain.h"
#   #include "world/Region.h"
#
target_include_directories(
    eldoria_world
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
