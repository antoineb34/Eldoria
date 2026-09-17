#pragma once

#include <cstdint>
#include <span>

#include "varp/VarpData.h"

namespace eld::varp {

class VarpDecoder {
public:
  VarpData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::varp
