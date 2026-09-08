#pragma once

#include <cstdint>
#include <span>

#include "map/TerrainData.h"

namespace eld::map {

class TerrainDecoder {
public:
  TerrainData decode(std::span<const std::uint8_t> payload,
                     std::uint16_t regionId) const;

private:
  static int generatedHeight(int worldX, int worldY);
};

} // namespace eld::map
