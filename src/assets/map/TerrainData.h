#pragma once

#include <array>
#include <cstddef>

#include "map/MapTile.h"

namespace eld::map {

struct TerrainData {
  std::array<MapTile, RegionTileCount> tiles{};

  MapTile &operator[](std::size_t index) { return tiles[index]; }

  const MapTile &operator[](std::size_t index) const { return tiles[index]; }

  MapTile &at(std::size_t index) { return tiles.at(index); }

  const MapTile &at(std::size_t index) const { return tiles.at(index); }

  const MapTile &tile(std::size_t plane, std::size_t x, std::size_t y) const {
    return at(tileIndex(plane, x, y));
  }
};

} // namespace eld::map
