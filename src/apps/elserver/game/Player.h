#pragma once

#include <cstdint>
#include <deque>
#include <string>

#include "world/Position.h"


namespace eld::elserver
{

    struct Player
    {
        std::uint32_t
            id = 1;


        std::string
            name = "Player";


        eld::world::TilePosition
            position{
                3232,
                3232,
                0
            };


        std::deque<
            eld::world::TilePosition
        >
            movementQueue;


        bool
            running = false;
    };

}
