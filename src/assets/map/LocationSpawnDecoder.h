#pragma once

#include <cstdint>
#include <span>

#include "map/MapLocationData.h"

namespace eld::map {

class LocationSpawnDecoder {
public:
  MapLocationData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::map
