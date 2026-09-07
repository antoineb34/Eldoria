#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "map/MapRegion.h"

namespace eld::map {

class LocationSpawnDecoder {
public:
    std::vector<MapLocationSpawn> decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
