#pragma once

#include <cstdint>
#include <span>

#include "location/LocationData.h"

namespace eld::location {

class LocationDecoder {
public:
  LocationData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::location
