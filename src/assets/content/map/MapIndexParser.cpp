#include "MapIndexParser.h"

#include <algorithm>
#include <cstddef>

#include "binary/ByteReader.h"

namespace eld::map {

std::optional<MapIndex> MapIndexParser::parse(
    const std::vector<std::uint8_t>& bytes
) const {
    constexpr std::size_t RecordSize = 7;

    if (
        bytes.empty() ||
        bytes.size() % RecordSize != 0
    ) {
        return std::nullopt;
    }

    eld::binary::ByteReader reader(bytes);

    MapIndex index;
    index.entries.reserve(
        bytes.size() / RecordSize
    );

    while (!reader.atEnd()) {
        MapIndexEntry entry;

        entry.regionId = reader.readU16();
        entry.terrainFileId = reader.readU16();
        entry.locationFileId = reader.readU16();
        entry.shouldPreload = reader.readU8() != 0;

        index.entries.push_back(entry);
    }

    std::sort(
        index.entries.begin(),
        index.entries.end(),
        [](const MapIndexEntry& a, const MapIndexEntry& b) {
            return a.regionId < b.regionId;
        }
    );

    const auto duplicate = std::adjacent_find(
        index.entries.begin(),
        index.entries.end(),
        [](const MapIndexEntry& a, const MapIndexEntry& b) {
            return a.regionId == b.regionId;
        }
    );

    if (duplicate != index.entries.end()) {
        return std::nullopt;
    }

    return index;
}

}
