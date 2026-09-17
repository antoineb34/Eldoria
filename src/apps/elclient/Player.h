#pragma once

#include "world/Position.h"

namespace eld::client {

enum class FacingDirection {
    North,
    East,
    South,
    West
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
