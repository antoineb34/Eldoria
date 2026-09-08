#pragma once

#include <cstdint>
#include <span>

#include "npc/NpcData.h"

namespace eld::npc {

class NpcDecoder {
public:
  NpcData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::npc
