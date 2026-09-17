#pragma once

#include "world/Position.h"

namespace eld::client {

enum class FacingDirection {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};


struct Player {
    eld::world::TilePosition tile{};

    eld::world::TileLocalPosition local{
        0.5f,
        0.5f
    };

    FacingDirection facing =
        FacingDirection::South;
};

}
