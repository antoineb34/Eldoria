#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "cache/Cache.h"
#include "cache/Store.h"
#include "decoders/LocationSpawnDecoder.h"
#include "decoders/MapIndexDecoder.h"
#include "decoders/TerrainDecoder.h"
#include "map/MapIndex.h"
#include "map/MapRegion.h"

namespace eld::map {

class MapRepository {
public:
    explicit MapRepository(
        const eld::cache::Cache& cache
    );

    MapRegion get(
        std::uint16_t regionId
    ) const;

    std::optional<MapRegion> find(
        std::uint16_t regionId
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t regionId
    ) const;

    std::size_t count() const;


    // Map-specific access

    MapRegion loadTerrain(
        std::uint16_t regionId
    ) const;

    const MapIndexEntry* findEntry(
        std::uint16_t regionId
    ) const;

    const std::vector<MapIndexEntry>&
    entries() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Maps;

    static constexpr auto ConfigIndex =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t VersionListArchiveId =
        5;

    static constexpr std::string_view MapIndexFile =
        "map_index";

    MapIndex readMapIndex(
        const eld::cache::Cache& cache
    ) const;

    const MapIndexEntry& requireEntry(
        std::uint16_t regionId
    ) const;

    eld::cache::Store maps_;
    MapIndex index_;

    MapIndexDecoder indexDecoder_;
    TerrainDecoder terrainDecoder_;
    LocationSpawnDecoder locationDecoder_;
};

}
