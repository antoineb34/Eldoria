#pragma once

#include <cstdint>
#include <optional>

#include "Position.h"

namespace eld::world
{

    using FloorId =
        std::uint16_t;

    struct TileSurface
    {
        std::optional<FloorId> underlay;
        std::optional<FloorId> overlay;

        // 0     = plain underlay
        // 1..12 = shaped surface
        std::uint8_t shape = 0;

        QuarterTurn rotation =
            QuarterTurn::Zero;
    };

    struct TileFlags
    {
        bool solid = false;
        bool bridge = false;
        bool roof = false;
    };

    struct Tile
    {
        TileSurface surface{};
        TileFlags flags{};

        // Original terrain layer from the map.
        std::uint8_t sourcePlane = 0;

        // Effective World plane after bridge projection.
        std::uint8_t scenePlane = 0;
    };

}
