#pragma once

#include "map/MapIndexData.h"
#include "map/MapLocationData.h"
#include "map/MapRegionResource.h"
#include "map/TerrainData.h"

namespace eld::map {

class MapAssembler {
public:
  MapRegionResource assemble(const MapIndexEntry &entry,
                             const TerrainData &terrain,
                             const MapLocationData &locations) const;
};

} // namespace eld::map
