#pragma once

#include <cstdint>
#include <vector>

#include "map/MapRegion.h"

namespace eld::map {

class ObjectSpawnDecoder {
public:
    std::vector<MapObjectSpawn> decode(
        const std::vector<std::uint8_t>& bytes
    ) const;
};

}
