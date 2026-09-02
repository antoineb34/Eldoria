#pragma once

#include <cstdint>
#include <vector>

#include "map/MapRegion.h"

namespace eld::map {

class LocationSpawnDecoder {
public:
    std::vector<MapLocationSpawn> decode(
        const std::vector<std::uint8_t>& bytes
    ) const;
};

}
