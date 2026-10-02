#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>


namespace eld::protocol
{

    inline constexpr std::size_t
        MaxPacketPayload =
            4096;


    struct Packet
    {
        std::uint8_t
            opcode = 0;


        std::vector<std::uint8_t>
            payload;
    };

}
