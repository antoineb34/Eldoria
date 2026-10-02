#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "Packet.h"


namespace eld::protocol
{

    class PacketDecoder
    {
    public:
        void append(
            std::span<const std::uint8_t> bytes
        );


        std::optional<Packet> next();


    private:
        std::vector<std::uint8_t>
            buffer_;
    };

}
