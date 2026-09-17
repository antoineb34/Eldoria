#pragma once

#include <cstdint>
#include <vector>

namespace eld::map {

struct MapLocationSpawn {
  std::uint16_t id = 0;
  std::uint8_t plane = 0;
  std::uint8_t x = 0;
  std::uint8_t y = 0;
  std::uint8_t type = 0;
  std::uint8_t rotation = 0;
};

using MapLocationData = std::vector<MapLocationSpawn>;

} // namespace eld::map
