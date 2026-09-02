#pragma once

#include <cstdint>
#include <vector>

#include "MapFileReader.h"
#include "MapIndex.h"
#include "MapRegion.h"
#include "cache/Cache.h"
#include "cache/Store.h"
#include "LocationSpawnDecoder.h"
#include "TerrainDecoder.h"

namespace eld::map {

class MapRepository {
public:
    explicit MapRepository(
        const eld::cache::Cache& cache
    );

    const std::vector<MapIndexEntry>& entries() const;

    const MapIndexEntry* find(
        std::uint16_t regionId
    ) const;

    MapRegion loadTerrain(
        std::uint16_t regionId
    ) const;

    MapRegion load(
        std::uint16_t regionId
    ) const;

private:
    MapIndex readMapIndex(
        const eld::cache::Cache& cache
    ) const;

    const MapIndexEntry& requireEntry(
        std::uint16_t regionId
    ) const;

private:
    eld::cache::Store maps_;
    MapIndex index_;

    MapFileReader fileReader_;
    TerrainDecoder terrainDecoder_;
    LocationSpawnDecoder locationDecoder_;
};

}
