#pragma once

#include <cstddef>
#include <cstdint>

#include "map/MapLocationData.h"
#include "map/TerrainData.h"

namespace eld::map {

struct MapRegionResource {
  std::uint16_t regionId = 0;
  std::uint16_t terrainFileId = 0;
  std::uint16_t locationFileId = 0;
  bool shouldPreload = false;

  TerrainData tiles;
  MapLocationData locations;

  int regionX() const { return static_cast<int>(regionId >> 8); }

  int regionY() const { return static_cast<int>(regionId & 0xFFu); }

  int worldBaseX() const { return regionX() * static_cast<int>(RegionSize); }

  int worldBaseY() const { return regionY() * static_cast<int>(RegionSize); }

  const MapTile &tile(std::size_t plane, std::size_t x, std::size_t y) const {
    return tiles.tile(plane, x, y);
  }
};

} // namespace eld::map
