#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "cache/Cache.h"
#include "map/LocationSpawnDecoder.h"
#include "map/MapIndexData.h"
#include "map/MapIndexDecoder.h"
#include "map/MapLocationData.h"
#include "map/TerrainData.h"
#include "map/TerrainDecoder.h"

namespace eld::map {

class MapLoader {
public:
    explicit MapLoader(
        const eld::cache::Cache& cache
    );

    const TerrainData& terrain(
        std::uint16_t regionId
    ) const;

    const MapLocationData& locations(
        std::uint16_t regionId
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t regionId
    ) const;

    std::size_t count() const;

    const MapIndexEntry* findEntry(
        std::uint16_t regionId
    ) const;

    const std::vector<MapIndexEntry>& entries() const;

private:
    static constexpr auto CacheIndex =
        eld::cache::IndexId::Maps;

    static constexpr auto ConfigIndex =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t
        VersionListArchiveId = 5;

    static constexpr std::string_view
        MapIndexFile = "map_index";

    MapIndexData readMapIndex(
        const eld::cache::Cache& cache
    ) const;

    const MapIndexEntry& requireEntry(
        std::uint16_t regionId
    ) const;

    TerrainData loadTerrain(
        const MapIndexEntry& entry
    ) const;

    MapLocationData loadLocations(
        const MapIndexEntry& entry
    ) const;

    eld::cache::Store maps_;
    MapIndexData index_;

    MapIndexDecoder indexDecoder_;
    TerrainDecoder terrainDecoder_;
    LocationSpawnDecoder locationDecoder_;

    mutable std::unordered_map<
        std::uint16_t,
        TerrainData
    > terrainCache_;

    mutable std::unordered_map<
        std::uint16_t,
        MapLocationData
    > locationCache_;
};

}
