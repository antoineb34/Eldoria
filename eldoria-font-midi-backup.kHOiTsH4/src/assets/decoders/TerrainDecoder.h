#pragma once

#include <cstdint>
#include <span>

#include "map/MapTile.h"

namespace eld::map {

class TerrainDecoder {
public:
    MapTileArray decode(
        std::span<const std::uint8_t> payload,
        std::uint16_t regionId
    ) const;

private:
    static int generatedHeight(
        int worldX,
        int worldY
    );
};

}
