#include "MapLoader.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

#include "archive/Archive.h"
#include "archive/ArchiveParser.h"
#include "binary/ByteReader.h"
#include "cache/File.h"

namespace eld::map {

MapLoader::MapLoader(
    const eld::cache::Cache& cache
)
    : maps_(cache.open(eld::cache::IndexId::Maps)),
      entries_(readMapIndex(cache)) {
}

const std::vector<MapIndexEntry>& MapLoader::entries() const {
    return entries_;
}

const MapIndexEntry* MapLoader::find(
    std::uint16_t regionId
) const {
    const auto it = std::lower_bound(
        entries_.begin(),
        entries_.end(),
        regionId,
        [](const MapIndexEntry& entry, std::uint16_t id) {
            return entry.regionId < id;
        }
    );

    return
        it != entries_.end() && it->regionId == regionId
            ? &*it
            : nullptr;
}

const MapIndexEntry& MapLoader::requireEntry(
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

MapRegion MapLoader::loadTerrain(
    std::uint16_t regionId
) const {
    const MapIndexEntry& entry = requireEntry(regionId);
    const MapFile terrainFile =
        fileReader_.read(maps_, entry.terrainFileId);

    MapRegion region;
    region.regionId = entry.regionId;
    region.terrainFileId = entry.terrainFileId;
    region.objectFileId = entry.objectFileId;
    region.shouldPreload = entry.shouldPreload;
    region.tiles =
        terrainDecoder_.decode(
            terrainFile.bytes,
            entry.regionId
        );
    return region;
}

MapRegion MapLoader::load(
    std::uint16_t regionId
) const {
    MapRegion region = loadTerrain(regionId);
    const MapFile objectFile =
        fileReader_.read(maps_, region.objectFileId);
    region.objects = objectDecoder_.decode(objectFile.bytes);
    return region;
}

std::vector<MapIndexEntry> MapLoader::readMapIndex(
    const eld::cache::Cache& cache
) const {
    const eld::cache::Store config =
        cache.open(eld::cache::IndexId::Config);

    if (!config.contains(5)) {
        throw std::runtime_error(
            "idx0 file 5 (versionlist) does not exist"
        );
    }

    const eld::cache::File versionListFile = config.get(5);
    const std::vector<std::uint8_t> archiveBytes =
        versionListFile.getBytes();

    const eld::archive::ArchiveParser parser;
    const std::optional<eld::archive::Archive> archive =
        parser.parse(archiveBytes);
    if (!archive.has_value()) {
        throw std::runtime_error(
            "idx0 file 5 is not a valid JAG archive"
        );
    }

    const eld::archive::ArchiveFile* mapIndex =
        archive->find("map_index");
    if (mapIndex == nullptr) {
        throw std::runtime_error(
            "versionlist archive has no map_index"
        );
    }

    constexpr std::size_t RecordSize = 7;
    const std::vector<std::uint8_t>& bytes = mapIndex->payload;
    if (bytes.empty() || bytes.size() % RecordSize != 0) {
        throw std::runtime_error(
            "map_index is not a non-empty sequence of 7-byte records"
        );
    }

    eld::binary::ByteReader reader(bytes);
    std::vector<MapIndexEntry> entries;
    entries.reserve(bytes.size() / RecordSize);

    while (!reader.atEnd()) {
        MapIndexEntry entry;
        entry.regionId = reader.readU16();
        entry.terrainFileId = reader.readU16();
        entry.objectFileId = reader.readU16();
        entry.shouldPreload = reader.readU8() != 0;
        entries.push_back(entry);
    }

    std::sort(
        entries.begin(),
        entries.end(),
        [](const MapIndexEntry& a, const MapIndexEntry& b) {
            return a.regionId < b.regionId;
        }
    );

    const auto duplicate = std::adjacent_find(
        entries.begin(),
        entries.end(),
        [](const MapIndexEntry& a, const MapIndexEntry& b) {
            return a.regionId == b.regionId;
        }
    );
    if (duplicate != entries.end()) {
        throw std::runtime_error(
            "map_index contains duplicate region ids"
        );
    }

    return entries;
}

}
