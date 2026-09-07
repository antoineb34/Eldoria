#include "repositories/MapRepository.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "archive/Archive.h"
#include "cache/File.h"

namespace eld::map {

MapRepository::MapRepository(
    const eld::cache::Cache& cache
)
    : maps_(
          cache.open(Index)
      ) {
    index_ = readMapIndex(cache);
}


MapRegion MapRepository::get(
    std::uint16_t regionId
) const {
    MapRegion region =
        loadTerrain(regionId);

    const eld::cache::File locationFile =
        maps_.get(
            region.locationFileId
        );

    const std::vector<std::uint8_t> locationData =
        locationFile.getBytes(
            eld::cache::CompressionState::Decompressed
        );

    try {
        region.locations =
            locationDecoder_.decode(
                locationData
            );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map locations for region " +
            std::to_string(regionId) +
            ": " +
            error.what()
        );
    }

    return region;
}


std::optional<MapRegion> MapRepository::find(
    std::uint16_t regionId
) const {
    if (!contains(regionId)) {
        return std::nullopt;
    }

    return get(regionId);
}


std::vector<std::uint16_t>
MapRepository::listIds() const {
    std::vector<std::uint16_t> ids;

    ids.reserve(
        index_.entries.size()
    );

    for (const MapIndexEntry& entry : index_.entries) {
        ids.push_back(
            entry.regionId
        );
    }

    return ids;
}


bool MapRepository::contains(
    std::uint16_t regionId
) const {
    return findEntry(regionId) != nullptr;
}


std::size_t MapRepository::count() const {
    return index_.entries.size();
}


MapRegion MapRepository::loadTerrain(
    std::uint16_t regionId
) const {
    const MapIndexEntry& entry =
        requireEntry(regionId);

    const eld::cache::File terrainFile =
        maps_.get(
            entry.terrainFileId
        );

    const std::vector<std::uint8_t> terrainData =
        terrainFile.getBytes(
            eld::cache::CompressionState::Decompressed
        );

    MapRegion region;

    region.regionId = entry.regionId;
    region.terrainFileId = entry.terrainFileId;
    region.locationFileId = entry.locationFileId;
    region.shouldPreload = entry.shouldPreload;

    try {
        region.tiles =
            terrainDecoder_.decode(
                terrainData,
                regionId
            );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map terrain for region " +
            std::to_string(regionId) +
            ": " +
            error.what()
        );
    }

    return region;
}


const MapIndexEntry*
MapRepository::findEntry(
    std::uint16_t regionId
) const {
    const auto entry =
        std::lower_bound(
            index_.entries.begin(),
            index_.entries.end(),
            regionId,
            [](
                const MapIndexEntry& candidate,
                std::uint16_t id
            ) {
                return candidate.regionId < id;
            }
        );

    return
        entry != index_.entries.end() &&
        entry->regionId == regionId
            ? &*entry
            : nullptr;
}


const std::vector<MapIndexEntry>&
MapRepository::entries() const {
    return index_.entries;
}


MapIndex MapRepository::readMapIndex(
    const eld::cache::Cache& cache
) const {
    const eld::archive::Archive archive =
        eld::archive::load(
            cache.open(ConfigIndex),
            VersionListArchiveId
        );

    const eld::archive::ArchiveFile& file =
        archive.get(
            MapIndexFile
        );

    try {
        return indexDecoder_.decode(
            file.payload
        );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map index: " +
            std::string(error.what())
        );
    }
}


const MapIndexEntry&
MapRepository::requireEntry(
    std::uint16_t regionId
) const {
    const MapIndexEntry* entry =
        findEntry(regionId);

    if (entry == nullptr) {
        throw std::out_of_range(
            "Map region does not exist: " +
            std::to_string(regionId)
        );
    }

    return *entry;
}

}
