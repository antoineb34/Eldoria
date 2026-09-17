#pragma once

#include <array>
#include <cstdint>

#include "Position.h"

namespace eld::world
{

    struct Location
    {
        std::uint16_t id = 0;

        // Tile occupied by this location.
        //
        // Coordinates are absolute World tile coordinates.
        // plane is the effective World plane.
        TilePosition tile{};

        // Resolved position in World units.
        WorldPosition position{};

        // Location shape/type from the map.
        std::uint8_t shape = 0;

        QuarterTurn rotation =
            QuarterTurn::Zero;

        // Rotation-aware size in tiles.
        int footprintWidth = 1;
        int footprintLength = 1;

        // Whether this placed location contributes to
        // walking collision.
        bool solid = false;

        // Terrain underneath this location:
        //
        // SW, SE, NE, NW
        //
        // Used by terrain-contoured location models.
        std::array<float, 4> groundHeights{};
    };

}
