#pragma once

#include <cstdint>
#include <span>
#include <vector>


namespace eld::protocol
{

    std::vector<std::uint8_t> framePacket(
        std::uint8_t opcode,
        std::span<const std::uint8_t> payload
    );

}
