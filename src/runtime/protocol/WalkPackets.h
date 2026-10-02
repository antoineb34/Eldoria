#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Packet.h"


namespace eld::protocol
{

    inline constexpr std::size_t
        MaxWalkSteps =
            512;


    struct WalkStep
    {
        std::int8_t
            dx = 0;


        std::int8_t
            dy = 0;
    };


    struct WalkRequest
    {
        std::uint16_t
            sourceX = 0;


        std::uint16_t
            sourceY = 0;


        std::uint8_t
            plane = 0;


        bool
            running = false;


        std::vector<WalkStep>
            steps;
    };


    std::vector<std::uint8_t> encode(
        const WalkRequest& packet
    );


    WalkRequest decodeWalkRequest(
        const Packet& packet
    );

}
