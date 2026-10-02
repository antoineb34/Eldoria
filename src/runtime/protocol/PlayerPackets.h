#pragma once

#include <cstdint>
#include <vector>

#include "Packet.h"


namespace eld::protocol
{

    struct PlayerPosition
    {
        std::uint16_t
            x = 0;


        std::uint16_t
            y = 0;


        std::uint8_t
            plane = 0;
    };


    std::vector<std::uint8_t> encode(
        const PlayerPosition& packet
    );


    PlayerPosition decodePlayerPosition(
        const Packet& packet
    );

}
