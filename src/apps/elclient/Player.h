#pragma once

#include "world/Position.h"

namespace eld::client {

struct Player {
    eld::world::TilePosition tile{};

    eld::world::TileLocalPosition local{
        0.5f,
        0.5f
    };
};

}
