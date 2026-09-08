#pragma once

#include <cstdint>
#include <span>

#include "Npc.h"

namespace eld::npc {

class NpcDecoder {
public:
    Npc decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
