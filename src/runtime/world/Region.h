#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "Location.h"
#include "Terrain.h"

namespace eld::world {

struct Region {
    std::uint16_t id = 0;

    Terrain terrain;

    std::vector<Location> locations;


    Region(
        std::uint16_t regionId,
        Terrain regionTerrain,
        std::vector<Location> regionLocations
    )
        : id(regionId),
          terrain(
              std::move(regionTerrain)
          ),
          locations(
              std::move(regionLocations)
          ) {
    }
};

}
