#pragma once

#include <cstdint>
#include <vector>

#include "map/MapTile.h"

namespace eld::map {

class TerrainDecoder {
public:
    MapTileArray decode(
        const std::vector<std::uint8_t>& bytes,
        std::uint16_t regionId
    ) const;

private:
    static int generatedHeight(
        int worldX,
        int worldY
    );
};

}
