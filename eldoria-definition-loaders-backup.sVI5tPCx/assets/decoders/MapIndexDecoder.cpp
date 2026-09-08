#include "decoders/MapIndexDecoder.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::map {

MapIndex MapIndexDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    constexpr std::size_t RecordSize = 7;

    if (payload.empty()) {
        throw std::runtime_error(
            "Map index is empty"
        );
    }

    if (payload.size() % RecordSize != 0) {
        throw std::runtime_error(
            "Map index has an invalid size"
        );
    }

    eld::binary::ByteReader reader(payload);

    MapIndex index;

    index.entries.reserve(
        payload.size() / RecordSize
    );


    // Entries

    while (!reader.atEnd()) {
        MapIndexEntry entry;

        entry.regionId = reader.readU16();
        entry.terrainFileId = reader.readU16();
        entry.locationFileId = reader.readU16();
        entry.shouldPreload = reader.readU8() != 0;

        index.entries.push_back(entry);
    }


    // Order

    std::sort(
        index.entries.begin(),
        index.entries.end(),
        [](const MapIndexEntry& a, const MapIndexEntry& b) {
            return a.regionId < b.regionId;
        }
    );

    const auto duplicate =
        std::adjacent_find(
            index.entries.begin(),
            index.entries.end(),
            [](const MapIndexEntry& a, const MapIndexEntry& b) {
                return a.regionId == b.regionId;
            }
        );

    if (duplicate != index.entries.end()) {
        throw std::runtime_error(
            "Map index contains duplicate regions"
        );
    }

    return index;
}

}
