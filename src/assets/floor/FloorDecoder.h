#pragma once

#include <cstdint>
#include <span>

#include "floor/FloorData.h"

namespace eld::floor {

class FloorDecoder {
public:
  FloorData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::floor
