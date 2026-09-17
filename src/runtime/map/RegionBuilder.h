#pragma once

#include <cstdint>

#include "location/LocationLoader.h"
#include "map/MapLoader.h"
#include "world/Region.h"

namespace eld::runtime::map {

class RegionBuilder {
public:
    eld::world::Region build(
        std::uint16_t regionId,
        const eld::map::MapLoader& maps,
        const eld::location::LocationLoader& locations
    ) const;
};

}
