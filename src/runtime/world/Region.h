#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "CollisionBuilder.h"
#include "Location.h"
#include "Terrain.h"

namespace eld::world
{

    struct Region
    {
        std::uint16_t id = 0;

        Terrain terrain;

        std::vector<Location> locations;

        CollisionMap collision;

        Region(
            std::uint16_t regionId,
            Terrain regionTerrain,
            std::vector<Location> regionLocations)
            : id(regionId),
              terrain(
                  std::move(regionTerrain)),
              locations(
                  std::move(regionLocations)),
              collision(
                  CollisionBuilder::build(
                      terrain,
                      locations))
        {
        }
    };

}
