#pragma once

#include <cstdint>
#include <span>

#include "map/MapIndexData.h"

namespace eld::map {

class MapIndexDecoder {
public:
  MapIndexData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::map
