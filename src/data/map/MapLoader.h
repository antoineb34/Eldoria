#pragma once

#include <cstdint>
#include <vector>

#include "MapFileReader.h"
#include "MapRegion.h"
#include "cache/Cache.h"
#include "cache/Store.h"
#include "decoder/ObjectSpawnDecoder.h"
#include "decoder/TerrainDecoder.h"

namespace eld::map {

class MapLoader {
public:
    explicit MapLoader(
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
    std::vector<MapIndexEntry> readMapIndex(
        const eld::cache::Cache& cache
    ) const;

    const MapIndexEntry& requireEntry(
        std::uint16_t regionId
    ) const;

private:
    eld::cache::Store maps_;
    std::vector<MapIndexEntry> entries_;

    MapFileReader fileReader_;
    TerrainDecoder terrainDecoder_;
    ObjectSpawnDecoder objectDecoder_;
};

}
