#include "MapRepository.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

#include "archive/Archive.h"
#include "archive/ArchiveParser.h"
#include "MapIndexParser.h"
#include "cache/File.h"

namespace eld::map {

MapRepository::MapRepository(
    const eld::cache::Cache& cache
)
    : maps_(cache.open(eld::cache::IndexId::Maps)),
      index_(readMapIndex(cache)) {
}

const std::vector<MapIndexEntry>& MapRepository::entries() const {
    return index_.entries;
}

const MapIndexEntry* MapRepository::find(
    std::uint16_t regionId
) const {
    const auto it = std::lower_bound(
        index_.entries.begin(),
        index_.entries.end(),
        regionId,
        [](const MapIndexEntry& entry, std::uint16_t id) {
            return entry.regionId < id;
        }
    );

    return
        it != index_.entries.end() && it->regionId == regionId
            ? &*it
            : nullptr;
}

const MapIndexEntry& MapRepository::requireEntry(
    std::uint16_t regionId
) const {
    const MapIndexEntry* entry = find(regionId);
    if (entry == nullptr) {
        throw std::out_of_range(
            "map_index has no region " +
            std::to_string(regionId)
        );
    }
    return *entry;
}

MapRegion MapRepository::loadTerrain(
    std::uint16_t regionId
) const {
    const MapIndexEntry& entry = requireEntry(regionId);
    const MapFile terrainFile =
        fileReader_.read(maps_, entry.terrainFileId);

    MapRegion region;
    region.regionId = entry.regionId;
    region.terrainFileId = entry.terrainFileId;
    region.locationFileId = entry.locationFileId;
    region.shouldPreload = entry.shouldPreload;
    region.tiles =
        terrainDecoder_.decode(
            terrainFile.bytes,
            entry.regionId
        );
    return region;
}

MapRegion MapRepository::load(
    std::uint16_t regionId
) const {
    MapRegion region = loadTerrain(regionId);
    const MapFile locationFile =
        fileReader_.read(maps_, region.locationFileId);
    region.locations = locationDecoder_.decode(locationFile.bytes);
    return region;
}

MapIndex MapRepository::readMapIndex(
    const eld::cache::Cache& cache
) const {
    const eld::cache::Store config =
        cache.open(eld::cache::IndexId::Config);

    if (!config.contains(5)) {
        throw std::runtime_error(
            "idx0 file 5 (versionlist) does not exist"
        );
    }

    const eld::cache::File versionListFile =
        config.get(5);

    const std::vector<std::uint8_t> archiveBytes =
        versionListFile.getBytes();

    const eld::archive::ArchiveParser archiveParser;

    const std::optional<eld::archive::Archive> archive =
        archiveParser.parse(archiveBytes);

    if (!archive.has_value()) {
        throw std::runtime_error(
            "idx0 file 5 is not a valid JAG archive"
        );
    }

    const eld::archive::ArchiveFile* mapIndexFile =
        archive->find("map_index");

    if (mapIndexFile == nullptr) {
        throw std::runtime_error(
            "versionlist archive has no map_index"
        );
    }

    const MapIndexParser parser;

    std::optional<MapIndex> index =
        parser.parse(mapIndexFile->payload);

    if (!index.has_value()) {
        throw std::runtime_error(
            "Failed to parse map_index"
        );
    }

    return std::move(*index);
}

}
