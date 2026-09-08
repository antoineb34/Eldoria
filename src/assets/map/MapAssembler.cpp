#include "map/MapAssembler.h"

namespace eld::map {

MapRegionResource
MapAssembler::assemble(const MapIndexEntry &entry, const TerrainData &terrain,
                       const MapLocationData &locations) const {
  MapRegionResource resource;

  resource.regionId = entry.regionId;
  resource.terrainFileId = entry.terrainFileId;
  resource.locationFileId = entry.locationFileId;
  resource.shouldPreload = entry.shouldPreload;
  resource.tiles = terrain;
  resource.locations = locations;

  return resource;
}

} // namespace eld::map
