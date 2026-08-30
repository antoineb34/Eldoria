#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "archive/ArchiveParser.h"
#include "binary/Compression.h"
#include "cache/Cache.h"
#include "cache/File.h"
#include "cache/Store.h"
#include "definition/DefinitionRepository.h"
#include "definition/floor/FloorRepository.h"
#include "definition/location/LocationRepository.h"

namespace {

constexpr std::size_t TerrainPlanes = 4;
constexpr std::size_t RegionSize = 64;
constexpr std::size_t TerrainTileCount =
    TerrainPlanes * RegionSize * RegionSize;

struct MapIndexEntry {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t objectFileId = 0;
    std::uint8_t shouldPreload = 0;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }
};

struct TerrainProbeResult {
    bool decodedAllTiles = false;
    bool exactSize = false;
    std::size_t consumed = 0;
    std::size_t tileCount = 0;

    std::size_t implicitHeightTiles = 0;
    std::size_t explicitHeightTiles = 0;
    std::size_t overlayOpcodes = 0;
    std::size_t settingOpcodes = 0;
    std::size_t underlayOpcodes = 0;

    std::array<std::size_t, 256> opcodeCounts{};
    std::string error;
};

struct ObjectPlacement {
    int objectId = -1;
    int plane = 0;
    int x = 0;
    int y = 0;
    int type = 0;
    int rotation = 0;
};

struct ObjectProbeResult {
    bool terminated = false;
    bool exactSize = false;
    bool classicTypesOnly = true;
    std::size_t consumed = 0;
    std::size_t objectIdGroups = 0;
    std::size_t placements = 0;
    int maxObjectId = -1;
    int maxType = -1;
    std::vector<ObjectPlacement> samples;
    std::vector<ObjectPlacement> decodedPlacements;
    std::string error;
};

struct MapIndexLoadResult {
    std::vector<MapIndexEntry> entries;
    std::string error;
};

struct LoadedFile {
    std::uint16_t id = 0;
    eld::cache::IndexEntry entry{};
    eld::binary::CompressionType compression =
        eld::binary::CompressionType::None;
    std::vector<std::uint8_t> storedBytes;
    std::vector<std::uint8_t> bytes;
};

struct TerrainPlaneDeepStats {
    std::size_t tiles = 0;
    std::size_t implicitHeights = 0;
    std::size_t explicitHeights = 0;
    int minHeight = std::numeric_limits<int>::max();
    int maxHeight = std::numeric_limits<int>::min();
    std::int64_t heightSum = 0;

    std::array<std::size_t, 256> overlayIds{};
    std::array<std::size_t, 256> underlayIds{};
    std::array<std::size_t, 33> settingValues{};
    std::array<std::size_t, 12> overlayShapes{};
    std::array<std::size_t, 4> overlayRotations{};
};

struct TerrainDeepResult {
    bool exactSize = false;
    std::size_t consumed = 0;
    std::array<TerrainPlaneDeepStats, TerrainPlanes> planes{};
    std::array<std::size_t, 256> overlayIds{};
    std::array<std::size_t, 256> underlayIds{};
    std::array<std::size_t, 33> settingValues{};
    std::string error;
};

struct ObjectDeepResult {
    std::array<std::size_t, TerrainPlanes> planeCounts{};
    std::array<std::size_t, 23> typeCounts{};
    std::array<std::size_t, 4> rotationCounts{};
    std::map<int, std::size_t> objectCounts;

    std::size_t definitionResolved = 0;
    std::size_t definitionMissing = 0;
    std::size_t definitionsWithNoModels = 0;
    std::size_t modelTypeCompatible = 0;
    std::size_t modelTypeIncompatible = 0;
    std::vector<ObjectPlacement> incompatibleSamples;
};

std::string compressionName(
    eld::binary::CompressionType type
) {
    switch (type) {
        case eld::binary::CompressionType::None:
            return "none";
        case eld::binary::CompressionType::Gzip:
            return "gzip";
        case eld::binary::CompressionType::Bzip2:
            return "bzip2";
    }

    return "unknown";
}

bool readU8(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& position,
    std::uint8_t& value
) {
    if (position >= bytes.size()) {
        return false;
    }

    value = bytes[position++];
    return true;
}

bool readU16(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& position,
    std::uint16_t& value
) {
    if (position + 2 > bytes.size()) {
        return false;
    }

    value =
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[position]) << 8
        ) |
        static_cast<std::uint16_t>(bytes[position + 1]);

    position += 2;
    return true;
}

bool readUnsignedSmart(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& position,
    std::uint32_t& value
) {
    if (position >= bytes.size()) {
        return false;
    }

    const std::uint8_t first =
        bytes[position];

    if (first < 128) {
        value = first;
        ++position;
        return true;
    }

    if (position + 2 > bytes.size()) {
        return false;
    }

    const std::uint16_t encoded =
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(bytes[position]) << 8
        ) |
        static_cast<std::uint16_t>(bytes[position + 1]);

    value =
        static_cast<std::uint32_t>(
            encoded - 32768u
        );

    position += 2;
    return true;
}


int classicNoise(
    int x,
    int y
) {
    int n = x + y * 57;
    n ^= n << 13;

    return
        (
            n *
            (n * n * 15731 + 789221) +
            1376312589
        ) &
        0x7fffffff;
}

int classicSmoothNoise(
    int x,
    int y
) {
    const int corners =
        classicNoise(x - 1, y - 1) +
        classicNoise(x + 1, y - 1) +
        classicNoise(x - 1, y + 1) +
        classicNoise(x + 1, y + 1);

    const int sides =
        classicNoise(x - 1, y) +
        classicNoise(x + 1, y) +
        classicNoise(x, y - 1) +
        classicNoise(x, y + 1);

    const int center =
        classicNoise(x, y);

    return
        corners / 16 +
        sides / 8 +
        center / 4;
}

int classicInterpolate(
    int a,
    int b,
    int fraction,
    int scale
) {
    int cosine =
        65536 -
        static_cast<int>(
            std::cos(
                static_cast<double>(fraction) *
                3.14159265358979323846 /
                static_cast<double>(scale)
            ) *
            65536.0
        );

    cosine >>= 1;

    return
        ((a * (65536 - cosine)) >> 16) +
        ((b * cosine) >> 16);
}

int classicInterpolatedNoise(
    int x,
    int y,
    int scale
) {
    const int scaledX = x / scale;
    const int localX = x & (scale - 1);
    const int scaledY = y / scale;
    const int localY = y & (scale - 1);

    const int v1 =
        classicSmoothNoise(scaledX, scaledY);
    const int v2 =
        classicSmoothNoise(scaledX + 1, scaledY);
    const int v3 =
        classicSmoothNoise(scaledX, scaledY + 1);
    const int v4 =
        classicSmoothNoise(scaledX + 1, scaledY + 1);

    const int i1 =
        classicInterpolate(v1, v2, localX, scale);
    const int i2 =
        classicInterpolate(v3, v4, localX, scale);

    return
        classicInterpolate(i1, i2, localY, scale);
}

int classicGeneratedHeight(
    int worldX,
    int worldY
) {
    int height =
        classicInterpolatedNoise(
            worldX + 45365,
            worldY + 91923,
            4
        ) -
        128;

    height +=
        (
            classicInterpolatedNoise(
                worldX + 10294,
                worldY + 37821,
                2
            ) -
            128
        ) >>
        1;

    height +=
        (
            classicInterpolatedNoise(
                worldX,
                worldY,
                1
            ) -
            128
        ) >>
        2;

    height =
        static_cast<int>(
            static_cast<double>(height) *
            0.3
        ) +
        35;

    return
        std::clamp(
            height,
            10,
            60
        );
}

TerrainDeepResult decodeTerrainDeep(
    const std::vector<std::uint8_t>& bytes,
    int regionX,
    int regionY
) {
    TerrainDeepResult result;
    std::size_t position = 0;
    std::array<int, TerrainTileCount> heights{};

    const int baseX = regionX * 64;
    const int baseY = regionY * 64;

    const auto tileIndex =
        [](std::size_t plane, std::size_t x, std::size_t y) {
            return
                plane * RegionSize * RegionSize +
                x * RegionSize +
                y;
        };

    for (
        std::size_t plane = 0;
        plane < TerrainPlanes;
        ++plane
    ) {
        TerrainPlaneDeepStats& planeStats =
            result.planes[plane];

        for (
            std::size_t x = 0;
            x < RegionSize;
            ++x
        ) {
            for (
                std::size_t y = 0;
                y < RegionSize;
                ++y
            ) {
                bool tileEnded = false;
                bool explicitHeight = false;
                bool implicitHeight = false;
                int tileHeight = 0;
                std::optional<std::uint8_t> overlayId;
                std::optional<std::uint8_t> underlayId;
                std::optional<std::uint8_t> setting;
                std::optional<std::uint8_t> overlayShape;
                std::optional<std::uint8_t> overlayRotation;

                while (!tileEnded) {
                    std::uint8_t opcode = 0;

                    if (!readU8(bytes, position, opcode)) {
                        std::ostringstream stream;
                        stream
                            << "EOF while deeply decoding terrain p="
                            << plane
                            << " x=" << x
                            << " y=" << y;
                        result.error = stream.str();
                        result.consumed = position;
                        return result;
                    }

                    if (opcode == 0) {
                        implicitHeight = true;

                        if (plane == 0) {
                            tileHeight =
                                -classicGeneratedHeight(
                                    baseX + static_cast<int>(x),
                                    baseY + static_cast<int>(y)
                                ) *
                                8;
                        }
                        else {
                            tileHeight =
                                heights[
                                    tileIndex(
                                        plane - 1,
                                        x,
                                        y
                                    )
                                ] -
                                240;
                        }

                        tileEnded = true;
                    }
                    else if (opcode == 1) {
                        std::uint8_t heightByte = 0;

                        if (!readU8(
                            bytes,
                            position,
                            heightByte
                        )) {
                            result.error =
                                "explicit terrain height is missing its byte";
                            result.consumed = position;
                            return result;
                        }

                        if (heightByte == 1) {
                            heightByte = 0;
                        }

                        explicitHeight = true;

                        if (plane == 0) {
                            tileHeight =
                                -static_cast<int>(heightByte) *
                                8;
                        }
                        else {
                            tileHeight =
                                heights[
                                    tileIndex(
                                        plane - 1,
                                        x,
                                        y
                                    )
                                ] -
                                static_cast<int>(heightByte) *
                                8;
                        }

                        tileEnded = true;
                    }
                    else if (opcode <= 49) {
                        std::uint8_t value = 0;

                        if (!readU8(bytes, position, value)) {
                            result.error =
                                "terrain overlay opcode is missing its id byte";
                            result.consumed = position;
                            return result;
                        }

                        overlayId = value;
                        overlayShape =
                            static_cast<std::uint8_t>(
                                (opcode - 2u) / 4u
                            );
                        overlayRotation =
                            static_cast<std::uint8_t>(
                                (opcode - 2u) & 3u
                            );
                    }
                    else if (opcode <= 81) {
                        setting =
                            static_cast<std::uint8_t>(
                                opcode - 49u
                            );
                    }
                    else {
                        underlayId =
                            static_cast<std::uint8_t>(
                                opcode - 81u
                            );
                    }
                }

                heights[tileIndex(plane, x, y)] =
                    tileHeight;

                ++planeStats.tiles;

                if (implicitHeight) {
                    ++planeStats.implicitHeights;
                }

                if (explicitHeight) {
                    ++planeStats.explicitHeights;
                }

                planeStats.minHeight =
                    std::min(
                        planeStats.minHeight,
                        tileHeight
                    );
                planeStats.maxHeight =
                    std::max(
                        planeStats.maxHeight,
                        tileHeight
                    );
                planeStats.heightSum +=
                    tileHeight;

                if (overlayId.has_value()) {
                    ++planeStats.overlayIds[*overlayId];
                    ++result.overlayIds[*overlayId];
                }

                if (underlayId.has_value()) {
                    ++planeStats.underlayIds[*underlayId];
                    ++result.underlayIds[*underlayId];
                }

                if (
                    setting.has_value() &&
                    *setting < planeStats.settingValues.size()
                ) {
                    ++planeStats.settingValues[*setting];
                    ++result.settingValues[*setting];
                }

                if (
                    overlayShape.has_value() &&
                    *overlayShape < planeStats.overlayShapes.size()
                ) {
                    ++planeStats.overlayShapes[*overlayShape];
                }

                if (overlayRotation.has_value()) {
                    ++planeStats.overlayRotations[*overlayRotation];
                }
            }
        }
    }

    result.consumed = position;
    result.exactSize =
        position == bytes.size();

    if (!result.exactSize) {
        std::ostringstream stream;
        stream
            << (bytes.size() - position)
            << " trailing byte(s) after deep terrain decode";
        result.error = stream.str();
    }

    return result;
}

int normalizedClassicLocationModelType(
    int placementType
) {
    if (placementType == 11) {
        return 10;
    }

    if (
        placementType >= 5 &&
        placementType <= 8
    ) {
        return 4;
    }

    return placementType;
}

bool locationDefinitionSupportsPlacementType(
    const eld::definition::LocationDefinition& definition,
    int placementType
) {
    if (definition.models.empty()) {
        return false;
    }

    const int wantedType =
        normalizedClassicLocationModelType(
            placementType
        );

    const bool hasTypedModels =
        std::any_of(
            definition.models.begin(),
            definition.models.end(),
            [](const eld::definition::LocationModel& model) {
                return model.type.has_value();
            }
        );

    if (!hasTypedModels) {
        return wantedType == 10;
    }

    return std::any_of(
        definition.models.begin(),
        definition.models.end(),
        [&](const eld::definition::LocationModel& model) {
            return
                model.type.has_value() &&
                static_cast<int>(*model.type) ==
                    wantedType;
        }
    );
}

ObjectDeepResult analyzeObjectsDeep(
    const ObjectProbeResult& objects,
    const eld::definition::LocationRepository& locations
) {
    ObjectDeepResult result;

    for (
        const ObjectPlacement& placement :
        objects.decodedPlacements
    ) {
        if (
            placement.plane >= 0 &&
            placement.plane <
                static_cast<int>(TerrainPlanes)
        ) {
            ++result.planeCounts[
                static_cast<std::size_t>(
                    placement.plane
                )
            ];
        }

        if (
            placement.type >= 0 &&
            placement.type <
                static_cast<int>(
                    result.typeCounts.size()
                )
        ) {
            ++result.typeCounts[
                static_cast<std::size_t>(
                    placement.type
                )
            ];
        }

        if (
            placement.rotation >= 0 &&
            placement.rotation < 4
        ) {
            ++result.rotationCounts[
                static_cast<std::size_t>(
                    placement.rotation
                )
            ];
        }

        ++result.objectCounts[placement.objectId];

        if (
            placement.objectId < 0 ||
            placement.objectId > 65535
        ) {
            ++result.definitionMissing;
            continue;
        }

        const eld::definition::LocationDefinition* definition =
            locations.find(
                static_cast<std::uint16_t>(
                    placement.objectId
                )
            );

        if (definition == nullptr) {
            ++result.definitionMissing;
            continue;
        }

        ++result.definitionResolved;

        if (definition->models.empty()) {
            ++result.definitionsWithNoModels;
            continue;
        }

        if (
            locationDefinitionSupportsPlacementType(
                *definition,
                placement.type
            )
        ) {
            ++result.modelTypeCompatible;
        }
        else {
            ++result.modelTypeIncompatible;

            if (
                result.incompatibleSamples.size() < 20
            ) {
                result.incompatibleSamples.push_back(
                    placement
                );
            }
        }
    }

    return result;
}

template <std::size_t N>
std::vector<std::pair<int, std::size_t>> topArrayCounts(
    const std::array<std::size_t, N>& counts,
    std::size_t limit
) {
    std::vector<std::pair<int, std::size_t>> values;

    for (std::size_t i = 0; i < counts.size(); ++i) {
        if (counts[i] == 0) {
            continue;
        }

        values.emplace_back(
            static_cast<int>(i),
            counts[i]
        );
    }

    std::sort(
        values.begin(),
        values.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }

            return a.first < b.first;
        }
    );

    if (values.size() > limit) {
        values.resize(limit);
    }

    return values;
}

std::vector<std::pair<int, std::size_t>> topMapCounts(
    const std::map<int, std::size_t>& counts,
    std::size_t limit
) {
    std::vector<std::pair<int, std::size_t>> values(
        counts.begin(),
        counts.end()
    );

    std::sort(
        values.begin(),
        values.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }

            return a.first < b.first;
        }
    );

    if (values.size() > limit) {
        values.resize(limit);
    }

    return values;
}

std::string locationModelTypeSummary(
    const eld::definition::LocationDefinition& definition
) {
    if (definition.models.empty()) {
        return "none";
    }

    std::ostringstream stream;
    bool first = true;

    for (
        const eld::definition::LocationModel& model :
        definition.models
    ) {
        if (!first) {
            stream << ',';
        }

        stream << model.id;

        if (model.type.has_value()) {
            stream
                << ":t"
                << static_cast<unsigned int>(
                       *model.type
                   );
        }
        else {
            stream << ":untyped";
        }

        first = false;
    }

    return stream.str();
}

std::string floorDefinitionSummary(
    const eld::definition::FloorDefinition* definition
) {
    if (definition == nullptr) {
        return "MISSING";
    }

    std::ostringstream stream;

    if (!definition->name.empty()) {
        stream << definition->name;
    }
    else {
        stream << "unnamed";
    }

    if (definition->rgb.has_value()) {
        stream
            << " rgb=#"
            << std::hex
            << std::setw(6)
            << std::setfill('0')
            << *definition->rgb
            << std::dec
            << std::setfill(' ');
    }

    if (definition->textureId.has_value()) {
        stream
            << " texture="
            << static_cast<unsigned int>(
                   *definition->textureId
               );
    }

    return stream.str();
}


TerrainProbeResult probeTerrain(
    const std::vector<std::uint8_t>& bytes
) {
    TerrainProbeResult result;
    std::size_t position = 0;

    for (
        std::size_t plane = 0;
        plane < TerrainPlanes;
        ++plane
    ) {
        for (
            std::size_t x = 0;
            x < RegionSize;
            ++x
        ) {
            for (
                std::size_t y = 0;
                y < RegionSize;
                ++y
            ) {
                bool tileEnded = false;

                while (!tileEnded) {
                    if (position >= bytes.size()) {
                        std::ostringstream stream;
                        stream
                            << "EOF at tile p=" << plane
                            << " x=" << x
                            << " y=" << y;
                        result.error = stream.str();
                        result.consumed = position;
                        return result;
                    }

                    const std::uint8_t opcode =
                        bytes[position++];

                    ++result.opcodeCounts[opcode];

                    if (opcode == 0) {
                        ++result.implicitHeightTiles;
                        tileEnded = true;
                    }
                    else if (opcode == 1) {
                        if (position >= bytes.size()) {
                            result.error =
                                "explicit-height opcode is missing its height byte";
                            result.consumed = position;
                            return result;
                        }

                        ++position;
                        ++result.explicitHeightTiles;
                        tileEnded = true;
                    }
                    else if (opcode <= 49) {
                        if (position >= bytes.size()) {
                            result.error =
                                "overlay opcode is missing its overlay id byte";
                            result.consumed = position;
                            return result;
                        }

                        ++position;
                        ++result.overlayOpcodes;
                    }
                    else if (opcode <= 81) {
                        ++result.settingOpcodes;
                    }
                    else {
                        ++result.underlayOpcodes;
                    }
                }

                ++result.tileCount;
            }
        }
    }

    result.decodedAllTiles =
        result.tileCount == TerrainTileCount;
    result.consumed = position;
    result.exactSize =
        result.decodedAllTiles &&
        result.consumed == bytes.size();

    if (
        result.decodedAllTiles &&
        !result.exactSize
    ) {
        std::ostringstream stream;
        stream
            << (bytes.size() - result.consumed)
            << " trailing byte(s) remain after 4x64x64 tiles";
        result.error = stream.str();
    }

    return result;
}

ObjectProbeResult probeObjects(
    const std::vector<std::uint8_t>& bytes
) {
    ObjectProbeResult result;
    std::size_t position = 0;
    int objectId = -1;

    while (true) {
        std::uint32_t idDelta = 0;

        if (
            !readUnsignedSmart(
                bytes,
                position,
                idDelta
            )
        ) {
            result.error =
                "EOF while reading object-id delta";
            result.consumed = position;
            return result;
        }

        if (idDelta == 0) {
            result.terminated = true;
            break;
        }

        if (
            idDelta >
            static_cast<std::uint32_t>(
                std::numeric_limits<int>::max() -
                objectId
            )
        ) {
            result.error =
                "object-id delta overflow";
            result.consumed = position;
            return result;
        }

        objectId +=
            static_cast<int>(idDelta);

        ++result.objectIdGroups;
        result.maxObjectId =
            std::max(
                result.maxObjectId,
                objectId
            );

        int location = 0;

        while (true) {
            std::uint32_t locationDelta = 0;

            if (
                !readUnsignedSmart(
                    bytes,
                    position,
                    locationDelta
                )
            ) {
                result.error =
                    "EOF while reading object-location delta";
                result.consumed = position;
                return result;
            }

            if (locationDelta == 0) {
                break;
            }

            if (
                locationDelta - 1u >
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::max() -
                    location
                )
            ) {
                result.error =
                    "object-location delta overflow";
                result.consumed = position;
                return result;
            }

            location +=
                static_cast<int>(
                    locationDelta - 1u
                );

            std::uint8_t attributes = 0;

            if (
                !readU8(
                    bytes,
                    position,
                    attributes
                )
            ) {
                result.error =
                    "EOF while reading object placement attributes";
                result.consumed = position;
                return result;
            }

            const int plane =
                (location >> 12) & 3;
            const int x =
                (location >> 6) & 63;
            const int y =
                location & 63;
            const int type =
                attributes >> 2;
            const int rotation =
                attributes & 3;

            result.maxType =
                std::max(
                    result.maxType,
                    type
                );

            if (type > 22) {
                result.classicTypesOnly = false;
            }

            const ObjectPlacement placement{
                objectId,
                plane,
                x,
                y,
                type,
                rotation
            };

            result.decodedPlacements.push_back(placement);

            if (result.samples.size() < 32) {
                result.samples.push_back(placement);
            }

            ++result.placements;
        }
    }

    result.consumed = position;
    result.exactSize =
        result.terminated &&
        result.consumed == bytes.size();

    if (
        result.terminated &&
        !result.exactSize
    ) {
        std::ostringstream stream;
        stream
            << (bytes.size() - result.consumed)
            << " trailing byte(s) remain after object terminator";
        result.error = stream.str();
    }

    return result;
}

MapIndexLoadResult loadMapIndex(
    const eld::cache::Cache& cache
) {
    MapIndexLoadResult result;

    try {
        const eld::cache::Store config =
            cache.open(
                eld::cache::IndexId::Config
            );

        if (!config.contains(5)) {
            result.error =
                "idx0 file 5 (versionlist) does not exist";
            return result;
        }

        const eld::cache::File versionListFile =
            config.get(5);

        const std::vector<std::uint8_t> archiveBytes =
            versionListFile.getBytes();

        const eld::archive::ArchiveParser parser;
        const std::optional<eld::archive::Archive> archive =
            parser.parse(archiveBytes);

        if (!archive.has_value()) {
            result.error =
                "idx0 file 5 could not be parsed as a JAG archive";
            return result;
        }

        const eld::archive::ArchiveFile* mapIndex =
            archive->find("map_index");

        if (mapIndex == nullptr) {
            result.error =
                "versionlist archive has no map_index file";
            return result;
        }

        const std::vector<std::uint8_t>& bytes =
            mapIndex->payload;

        // This revision stores map_index as a flat sequence of 7-byte
        // records. There is no leading entry count. The first two bytes are
        // the first region id (the previous probe interpreted that region id
        // as a count, which is why 0x1d4b became the impossible count 7499).
        constexpr std::size_t MapIndexEntrySize = 7;

        if (
            bytes.empty() ||
            bytes.size() % MapIndexEntrySize != 0
        ) {
            std::ostringstream stream;
            stream
                << "map_index size "
                << bytes.size()
                << " is not a non-empty multiple of "
                << MapIndexEntrySize;
            result.error = stream.str();
            return result;
        }

        const std::size_t count =
            bytes.size() / MapIndexEntrySize;
        std::size_t position = 0;

        result.entries.reserve(count);

        for (
            std::size_t index = 0;
            index < count;
            ++index
        ) {
            MapIndexEntry entry;

            if (
                !readU16(
                    bytes,
                    position,
                    entry.regionId
                ) ||
                !readU16(
                    bytes,
                    position,
                    entry.terrainFileId
                ) ||
                !readU16(
                    bytes,
                    position,
                    entry.objectFileId
                ) ||
                !readU8(
                    bytes,
                    position,
                    entry.shouldPreload
                )
            ) {
                result.entries.clear();
                result.error =
                    "map_index ended in the middle of an entry";
                return result;
            }

            result.entries.push_back(entry);
        }

        if (position != bytes.size()) {
            result.entries.clear();
            result.error =
                "map_index parser did not consume the complete file";
            return result;
        }
    }
    catch (const std::exception& exception) {
        result.entries.clear();
        result.error =
            std::string("map_index load failed: ") +
            exception.what();
    }

    return result;
}

LoadedFile loadFile(
    const eld::cache::Store& maps,
    std::uint16_t fileId
) {
    const eld::cache::File file =
        maps.get(fileId);

    LoadedFile loaded;
    loaded.id = fileId;
    loaded.entry = file.getEntry();
    loaded.compression =
        file.getCompressionType();
    loaded.storedBytes =
        file.getBytes(
            eld::cache::CompressionState::Compressed
        );
    loaded.bytes =
        file.getBytes(
            eld::cache::CompressionState::Decompressed
        );

    return loaded;
}

double byteEntropy(
    const std::vector<std::uint8_t>& bytes
) {
    if (bytes.empty()) {
        return 0.0;
    }

    std::array<std::size_t, 256> counts{};

    for (const std::uint8_t byte : bytes) {
        ++counts[byte];
    }

    double entropy = 0.0;
    const double size =
        static_cast<double>(bytes.size());

    for (const std::size_t count : counts) {
        if (count == 0) {
            continue;
        }

        const double probability =
            static_cast<double>(count) /
            size;

        entropy -=
            probability *
            std::log2(probability);
    }

    return entropy;
}

std::string hexBytes(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t count
) {
    if (
        offset >= bytes.size() ||
        count == 0
    ) {
        return "(none)";
    }

    const std::size_t end =
        std::min(
            bytes.size(),
            offset + count
        );

    std::ostringstream stream;
    stream
        << std::hex
        << std::setfill('0');

    for (
        std::size_t index = offset;
        index < end;
        ++index
    ) {
        if (index != offset) {
            stream << ' ';
        }

        stream
            << std::setw(2)
            << static_cast<unsigned int>(
                   bytes[index]
               );
    }

    return stream.str();
}

std::vector<const MapIndexEntry*> referencesForFile(
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t fileId
) {
    std::vector<const MapIndexEntry*> result;

    for (const MapIndexEntry& entry : mapIndex) {
        if (
            entry.terrainFileId == fileId ||
            entry.objectFileId == fileId
        ) {
            result.push_back(&entry);
        }
    }

    return result;
}

void printMapIndexEntry(
    const MapIndexEntry& entry,
    const eld::cache::Store& maps
) {
    std::cout
        << "region=" << entry.regionId
        << " (" << entry.regionX()
        << "," << entry.regionY() << ")"
        << " terrain=" << entry.terrainFileId
        << (maps.contains(entry.terrainFileId)
                ? ""
                : " [MISSING]")
        << " objects=" << entry.objectFileId
        << (maps.contains(entry.objectFileId)
                ? ""
                : " [MISSING]")
        << " preload="
        << static_cast<unsigned int>(
               entry.shouldPreload
           )
        << '\n';
}

void printSummary(
    const eld::cache::Cache& cache,
    const eld::cache::Store& maps,
    const MapIndexLoadResult& mapIndexResult
) {
    const std::vector<eld::cache::FileEntry> files =
        maps.list();

    std::uint64_t totalStoredBytes = 0;
    std::uint32_t smallest =
        std::numeric_limits<std::uint32_t>::max();
    std::uint32_t largest = 0;

    for (const eld::cache::FileEntry& file : files) {
        totalStoredBytes +=
            file.indexEntry.size;
        smallest =
            std::min(
                smallest,
                file.indexEntry.size
            );
        largest =
            std::max(
                largest,
                file.indexEntry.size
            );
    }

    if (files.empty()) {
        smallest = 0;
    }

    std::cout
        << "INDEX 4 / MAPS\n"
        << "==============\n"
        << "entries: " << files.size() << '\n'
        << "stored bytes: " << totalStoredBytes << '\n'
        << "smallest file: " << smallest << '\n'
        << "largest file: " << largest << '\n';

    if (!mapIndexResult.error.empty()) {
        std::cout
            << "\nmap_index: NOT VERIFIED\n"
            << mapIndexResult.error
            << '\n';
        return;
    }

    std::size_t missingTerrain = 0;
    std::size_t missingObjects = 0;

    for (
        const MapIndexEntry& entry :
        mapIndexResult.entries
    ) {
        if (!maps.contains(entry.terrainFileId)) {
            ++missingTerrain;
        }

        if (!maps.contains(entry.objectFileId)) {
            ++missingObjects;
        }
    }

    std::cout
        << "\nmap_index: VERIFIED STRUCTURE\n"
        << "regions: "
        << mapIndexResult.entries.size()
        << '\n'
        << "missing terrain refs: "
        << missingTerrain
        << '\n'
        << "missing object refs: "
        << missingObjects
        << '\n'
        << "\nfirst regions:\n";

    const std::size_t previewCount =
        std::min<std::size_t>(
            20,
            mapIndexResult.entries.size()
        );

    for (
        std::size_t index = 0;
        index < previewCount;
        ++index
    ) {
        printMapIndexEntry(
            mapIndexResult.entries[index],
            maps
        );
    }

    std::cout
        << "\nNext useful commands:\n"
        << "  map_probe <cache> scan\n"
        << "  map_probe <cache> regions\n"
        << "  map_probe <cache> file <file-id>\n"
        << "  map_probe <cache> region <region-id>\n"
        << "  map_probe <cache> region <region-x> <region-y>\n"
        << "  map_probe <cache> deep <region-id|x y>\n"
        << "  map_probe <cache> validate\n";
}

void printFileDetail(
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t fileId
) {
    if (!maps.contains(fileId)) {
        std::cerr
            << "Index 4 has no file "
            << fileId
            << '\n';
        return;
    }

    const LoadedFile file =
        loadFile(maps, fileId);

    const TerrainProbeResult terrain =
        probeTerrain(file.bytes);
    const ObjectProbeResult objects =
        probeObjects(file.bytes);

    const std::size_t zeroCount =
        static_cast<std::size_t>(
            std::count(
                file.bytes.begin(),
                file.bytes.end(),
                std::uint8_t{0}
            )
        );

    const double zeroPercent =
        file.bytes.empty()
            ? 0.0
            : static_cast<double>(zeroCount) *
                100.0 /
                static_cast<double>(
                    file.bytes.size()
                );

    std::cout
        << "INDEX 4 FILE " << fileId << '\n'
        << "=================\n"
        << "stored size: "
        << file.storedBytes.size() << '\n'
        << "index size: "
        << file.entry.size << '\n'
        << "first sector: "
        << file.entry.firstSector << '\n'
        << "compression: "
        << compressionName(file.compression)
        << '\n'
        << "decompressed size: "
        << file.bytes.size() << '\n'
        << std::fixed
        << std::setprecision(2)
        << "zero bytes: "
        << zeroPercent << "%\n"
        << "entropy: "
        << byteEntropy(file.bytes)
        << " bits/byte\n";

    const auto references =
        referencesForFile(
            mapIndex,
            fileId
        );

    if (!references.empty()) {
        std::cout
            << "\nmap_index references:\n";

        for (
            const MapIndexEntry* reference :
            references
        ) {
            const bool terrainRole =
                reference->terrainFileId ==
                fileId;
            const bool objectRole =
                reference->objectFileId ==
                fileId;

            std::cout
                << "  region="
                << reference->regionId
                << " ("
                << reference->regionX()
                << ","
                << reference->regionY()
                << ") role=";

            if (terrainRole && objectRole) {
                std::cout << "terrain+objects";
            }
            else if (terrainRole) {
                std::cout << "terrain";
            }
            else {
                std::cout << "objects";
            }

            std::cout
                << " preload="
                << static_cast<unsigned int>(
                       reference->shouldPreload
                   )
                << '\n';
        }
    }
    else {
        std::cout
            << "\nmap_index references: none\n";
    }

    std::cout
        << "\nterrain grammar probe:\n"
        << "  decoded tiles: "
        << terrain.tileCount
        << "/"
        << TerrainTileCount
        << '\n'
        << "  consumed: "
        << terrain.consumed
        << "/"
        << file.bytes.size()
        << '\n'
        << "  exact: "
        << (terrain.exactSize
                ? "YES"
                : "no")
        << '\n'
        << "  opcode0 implicit height: "
        << terrain.implicitHeightTiles
        << '\n'
        << "  opcode1 explicit height: "
        << terrain.explicitHeightTiles
        << '\n'
        << "  overlay opcodes: "
        << terrain.overlayOpcodes
        << '\n'
        << "  settings opcodes: "
        << terrain.settingOpcodes
        << '\n'
        << "  underlay opcodes: "
        << terrain.underlayOpcodes
        << '\n';

    if (!terrain.error.empty()) {
        std::cout
            << "  note: "
            << terrain.error
            << '\n';
    }

    std::cout
        << "\nobject smart-delta probe:\n"
        << "  terminated: "
        << (objects.terminated
                ? "yes"
                : "no")
        << '\n'
        << "  consumed: "
        << objects.consumed
        << "/"
        << file.bytes.size()
        << '\n'
        << "  exact: "
        << (objects.exactSize
                ? "YES"
                : "no")
        << '\n'
        << "  object id groups: "
        << objects.objectIdGroups
        << '\n'
        << "  placements: "
        << objects.placements
        << '\n'
        << "  max object id: "
        << objects.maxObjectId
        << '\n'
        << "  max placement type: "
        << objects.maxType
        << '\n'
        << "  classic types <=22: "
        << (objects.classicTypesOnly
                ? "yes"
                : "NO")
        << '\n';

    if (!objects.error.empty()) {
        std::cout
            << "  note: "
            << objects.error
            << '\n';
    }

    if (!objects.samples.empty()) {
        std::cout
            << "  first placements:\n";

        for (
            const ObjectPlacement& placement :
            objects.samples
        ) {
            std::cout
                << "    obj="
                << placement.objectId
                << " p="
                << placement.plane
                << " x="
                << placement.x
                << " y="
                << placement.y
                << " type="
                << placement.type
                << " rot="
                << placement.rotation
                << '\n';
        }
    }

    std::cout
        << "\nfirst 128 decompressed bytes:\n"
        << hexBytes(
               file.bytes,
               0,
               128
           )
        << '\n';

    if (file.bytes.size() > 128) {
        const std::size_t tailCount =
            std::min<std::size_t>(
                64,
                file.bytes.size()
            );

        std::cout
            << "\nlast "
            << tailCount
            << " decompressed bytes:\n"
            << hexBytes(
                   file.bytes,
                   file.bytes.size() -
                       tailCount,
                   tailCount
               )
            << '\n';
    }
}

void scanAll(
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex
) {
    const std::vector<eld::cache::FileEntry> files =
        maps.list();

    std::size_t terrainExact = 0;
    std::size_t objectExact = 0;
    std::size_t bothExact = 0;
    std::size_t unknown = 0;
    std::size_t loadFailures = 0;
    std::size_t mapIndexRoleMismatches = 0;

    std::cout
        << "file  stored  raw  compression  terrain  objects  expected\n";

    for (const eld::cache::FileEntry& entry : files) {
        try {
            const LoadedFile file =
                loadFile(
                    maps,
                    entry.fileId
                );

            const TerrainProbeResult terrain =
                probeTerrain(file.bytes);
            const ObjectProbeResult objects =
                probeObjects(file.bytes);

            const bool isTerrain =
                terrain.exactSize;
            const bool isObjects =
                objects.exactSize &&
                objects.classicTypesOnly;

            if (isTerrain && isObjects) {
                ++bothExact;
            }
            else if (isTerrain) {
                ++terrainExact;
            }
            else if (isObjects) {
                ++objectExact;
            }
            else {
                ++unknown;
            }

            bool expectedTerrain = false;
            bool expectedObjects = false;

            for (
                const MapIndexEntry& mapEntry :
                mapIndex
            ) {
                if (
                    mapEntry.terrainFileId ==
                    entry.fileId
                ) {
                    expectedTerrain = true;
                }

                if (
                    mapEntry.objectFileId ==
                    entry.fileId
                ) {
                    expectedObjects = true;
                }
            }

            const bool roleMismatch =
                (expectedTerrain && !isTerrain) ||
                (expectedObjects && !isObjects);

            if (roleMismatch) {
                ++mapIndexRoleMismatches;
            }

            std::string expected = "-";

            if (
                expectedTerrain &&
                expectedObjects
            ) {
                expected = "terrain+objects";
            }
            else if (expectedTerrain) {
                expected = "terrain";
            }
            else if (expectedObjects) {
                expected = "objects";
            }

            std::cout
                << entry.fileId
                << "  "
                << file.storedBytes.size()
                << "  "
                << file.bytes.size()
                << "  "
                << compressionName(
                       file.compression
                   )
                << "  "
                << (isTerrain ? "YES" : "-")
                << "  "
                << (isObjects ? "YES" : "-")
                << "  "
                << expected;

            if (roleMismatch) {
                std::cout << "  <-- MISMATCH";
            }

            if (
                !isTerrain &&
                terrain.decodedAllTiles
            ) {
                std::cout
                    << "  terrain+"
                    << (
                        file.bytes.size() >=
                                terrain.consumed
                            ? file.bytes.size() -
                                terrain.consumed
                            : 0
                    )
                    << " trailing";
            }

            if (
                objects.terminated &&
                !objects.exactSize
            ) {
                std::cout
                    << "  objects+"
                    << (
                        file.bytes.size() >=
                                objects.consumed
                            ? file.bytes.size() -
                                objects.consumed
                            : 0
                    )
                    << " trailing";
            }

            std::cout << '\n';
        }
        catch (const std::exception& exception) {
            ++loadFailures;

            std::cout
                << entry.fileId
                << "  "
                << entry.indexEntry.size
                << "  ?  ?  ?  ?  ?  ERROR: "
                << exception.what()
                << '\n';
        }
    }

    std::cout
        << "\nSCAN SUMMARY\n"
        << "============\n"
        << "terrain-only exact: "
        << terrainExact
        << '\n'
        << "objects-only exact: "
        << objectExact
        << '\n'
        << "both grammars exact: "
        << bothExact
        << '\n'
        << "unknown: "
        << unknown
        << '\n'
        << "load failures: "
        << loadFailures
        << '\n'
        << "map_index role mismatches: "
        << mapIndexRoleMismatches
        << '\n';
}

void printRegions(
    const eld::cache::Store& maps,
    const MapIndexLoadResult& mapIndexResult
) {
    if (!mapIndexResult.error.empty()) {
        std::cerr
            << "Cannot list regions: "
            << mapIndexResult.error
            << '\n';
        return;
    }

    std::cout
        << "MAP INDEX REGIONS\n"
        << "=================\n";

    for (
        const MapIndexEntry& entry :
        mapIndexResult.entries
    ) {
        printMapIndexEntry(
            entry,
            maps
        );
    }

    std::cout
        << "\nregions: "
        << mapIndexResult.entries.size()
        << '\n';
}

const MapIndexEntry* findRegion(
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t regionId
) {
    const auto found =
        std::find_if(
            mapIndex.begin(),
            mapIndex.end(),
            [&](const MapIndexEntry& entry) {
                return
                    entry.regionId ==
                    regionId;
            }
        );

    return
        found == mapIndex.end()
            ? nullptr
            : &*found;
}

void printRegionDetail(
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t regionId
) {
    const MapIndexEntry* region =
        findRegion(
            mapIndex,
            regionId
        );

    if (region == nullptr) {
        std::cerr
            << "map_index has no region "
            << regionId
            << '\n';
        return;
    }

    std::cout
        << "REGION "
        << region->regionId
        << " ("
        << region->regionX()
        << ","
        << region->regionY()
        << ")\n"
        << "terrain file: "
        << region->terrainFileId
        << '\n'
        << "object file: "
        << region->objectFileId
        << '\n'
        << "preload: "
        << static_cast<unsigned int>(
               region->shouldPreload
           )
        << "\n\n";

    if (maps.contains(region->terrainFileId)) {
        const LoadedFile terrainFile =
            loadFile(
                maps,
                region->terrainFileId
            );

        const TerrainProbeResult terrain =
            probeTerrain(
                terrainFile.bytes
            );

        std::cout
            << "terrain parse: "
            << (
                terrain.exactSize
                    ? "EXACT"
                    : "FAILED"
            )
            << " raw="
            << terrainFile.bytes.size()
            << " stored="
            << terrainFile.storedBytes.size()
            << " compression="
            << compressionName(
                   terrainFile.compression
               )
            << '\n'
            << "  implicit heights="
            << terrain.implicitHeightTiles
            << " explicit heights="
            << terrain.explicitHeightTiles
            << " overlays="
            << terrain.overlayOpcodes
            << " settings="
            << terrain.settingOpcodes
            << " underlays="
            << terrain.underlayOpcodes
            << '\n';

        if (!terrain.error.empty()) {
            std::cout
                << "  note: "
                << terrain.error
                << '\n';
        }
    }
    else {
        std::cout
            << "terrain parse: FILE MISSING\n";
    }

    if (maps.contains(region->objectFileId)) {
        const LoadedFile objectFile =
            loadFile(
                maps,
                region->objectFileId
            );

        const ObjectProbeResult objects =
            probeObjects(
                objectFile.bytes
            );

        std::cout
            << "\nobject parse: "
            << (
                objects.exactSize &&
                objects.classicTypesOnly
                    ? "EXACT"
                    : "FAILED"
            )
            << " raw="
            << objectFile.bytes.size()
            << " stored="
            << objectFile.storedBytes.size()
            << " compression="
            << compressionName(
                   objectFile.compression
               )
            << '\n'
            << "  groups="
            << objects.objectIdGroups
            << " placements="
            << objects.placements
            << " maxObjectId="
            << objects.maxObjectId
            << " maxType="
            << objects.maxType
            << '\n';

        if (!objects.error.empty()) {
            std::cout
                << "  note: "
                << objects.error
                << '\n';
        }

        if (!objects.samples.empty()) {
            std::cout
                << "  first placements:\n";

            for (
                const ObjectPlacement& placement :
                objects.samples
            ) {
                std::cout
                    << "    obj="
                    << placement.objectId
                    << " p="
                    << placement.plane
                    << " x="
                    << placement.x
                    << " y="
                    << placement.y
                    << " type="
                    << placement.type
                    << " rot="
                    << placement.rotation
                    << '\n';
            }
        }
    }
    else {
        std::cout
            << "\nobject parse: FILE MISSING\n";
    }
}


struct FloorReferenceValidation {
    std::size_t valid = 0;
    std::size_t invalid = 0;
    std::size_t zero = 0;
};

FloorReferenceValidation validateFloorReferences(
    const std::array<std::size_t, 256>& counts,
    const eld::definition::FloorRepository& floors
) {
    FloorReferenceValidation result;

    result.zero = counts[0];

    for (
        std::size_t rawId = 1;
        rawId < counts.size();
        ++rawId
    ) {
        const std::size_t count =
            counts[rawId];

        if (count == 0) {
            continue;
        }

        const std::uint16_t floorId =
            static_cast<std::uint16_t>(
                rawId - 1
            );

        if (floors.find(floorId) != nullptr) {
            result.valid += count;
        }
        else {
            result.invalid += count;
        }
    }

    return result;
}

void printTopFloorReferences(
    std::string_view label,
    const std::array<std::size_t, 256>& counts,
    const eld::definition::FloorRepository& floors
) {
    const FloorReferenceValidation validation =
        validateFloorReferences(
            counts,
            floors
        );

    std::cout
        << label
        << " floor refs: valid="
        << validation.valid
        << " invalid="
        << validation.invalid
        << " raw-zero="
        << validation.zero
        << '\n';

    const auto top =
        topArrayCounts(
            counts,
            12
        );

    for (const auto& [rawId, count] : top) {
        if (rawId == 0) {
            std::cout
                << "  raw=0 count="
                << count
                << " (no floor id)\n";
            continue;
        }

        const std::uint16_t floorId =
            static_cast<std::uint16_t>(
                rawId - 1
            );

        std::cout
            << "  raw="
            << rawId
            << " -> flo["
            << floorId
            << "] count="
            << count
            << " "
            << floorDefinitionSummary(
                   floors.find(floorId)
               )
            << '\n';
    }
}

void printRegionDeep(
    const eld::cache::Cache& cache,
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t regionId
) {
    const MapIndexEntry* region =
        findRegion(
            mapIndex,
            regionId
        );

    if (region == nullptr) {
        std::cerr
            << "map_index has no region "
            << regionId
            << '\n';
        return;
    }

    const eld::definition::DefinitionRepository definitions(
        cache.open(eld::cache::IndexId::Config),
        2
    );
    const eld::definition::FloorRepository floors(
        definitions.get("flo")
    );
    const eld::definition::LocationRepository locations(
        definitions.get("loc")
    );

    std::cout
        << "DEEP REGION "
        << region->regionId
        << " ("
        << region->regionX()
        << ","
        << region->regionY()
        << ")\n"
        << "world base: x="
        << region->regionX() * 64
        << " y="
        << region->regionY() * 64
        << '\n'
        << "terrain="
        << region->terrainFileId
        << " objects="
        << region->objectFileId
        << " preload="
        << static_cast<unsigned int>(region->shouldPreload)
        << "\n\n";

    if (!maps.contains(region->terrainFileId)) {
        std::cout
            << "terrain: FILE MISSING\n";
    }
    else {
        const LoadedFile terrainFile =
            loadFile(
                maps,
                region->terrainFileId
            );

        const TerrainDeepResult terrain =
            decodeTerrainDeep(
                terrainFile.bytes,
                region->regionX(),
                region->regionY()
            );

        std::cout
            << "TERRAIN SEMANTICS\n"
            << "-----------------\n"
            << "decode: "
            << (terrain.exactSize ? "EXACT" : "FAILED")
            << " raw="
            << terrainFile.bytes.size()
            << " stored="
            << terrainFile.storedBytes.size()
            << '\n';

        if (!terrain.error.empty()) {
            std::cout
                << "note: "
                << terrain.error
                << '\n';
        }

        for (
            std::size_t plane = 0;
            plane < TerrainPlanes;
            ++plane
        ) {
            const TerrainPlaneDeepStats& stats =
                terrain.planes[plane];

            const double averageHeight =
                stats.tiles == 0
                    ? 0.0
                    : static_cast<double>(
                          stats.heightSum
                      ) /
                      static_cast<double>(
                          stats.tiles
                      );

            const std::size_t overlays =
                std::accumulate(
                    stats.overlayIds.begin(),
                    stats.overlayIds.end(),
                    std::size_t{0}
                );
            const std::size_t underlays =
                std::accumulate(
                    stats.underlayIds.begin(),
                    stats.underlayIds.end(),
                    std::size_t{0}
                );
            const std::size_t settings =
                std::accumulate(
                    stats.settingValues.begin(),
                    stats.settingValues.end(),
                    std::size_t{0}
                );

            std::cout
                << "plane "
                << plane
                << ": height="
                << stats.minHeight
                << ".."
                << stats.maxHeight
                << " avg="
                << std::fixed
                << std::setprecision(2)
                << averageHeight
                << std::defaultfloat
                << " implicit="
                << stats.implicitHeights
                << " explicit="
                << stats.explicitHeights
                << " overlays="
                << overlays
                << " underlays="
                << underlays
                << " settings="
                << settings
                << '\n';

            const auto shapes =
                topArrayCounts(
                    stats.overlayShapes,
                    stats.overlayShapes.size()
                );

            if (!shapes.empty()) {
                std::cout
                    << "  overlay shapes:";

                for (const auto& [shape, count] : shapes) {
                    std::cout
                        << " "
                        << shape
                        << "="
                        << count;
                }

                std::cout << '\n';
            }
        }

        const auto settings =
            topArrayCounts(
                terrain.settingValues,
                terrain.settingValues.size()
            );

        std::cout
            << "settings values:";

        if (settings.empty()) {
            std::cout << " none";
        }
        else {
            for (const auto& [value, count] : settings) {
                std::cout
                    << " "
                    << value
                    << "="
                    << count;
            }
        }

        std::cout << "\n\n";

        printTopFloorReferences(
            "overlay",
            terrain.overlayIds,
            floors
        );
        printTopFloorReferences(
            "underlay",
            terrain.underlayIds,
            floors
        );
    }

    std::cout << '\n';

    if (!maps.contains(region->objectFileId)) {
        std::cout
            << "objects: FILE MISSING\n";
        return;
    }

    const LoadedFile objectFile =
        loadFile(
            maps,
            region->objectFileId
        );
    const ObjectProbeResult objects =
        probeObjects(
            objectFile.bytes
        );

    std::cout
        << "OBJECT SEMANTICS\n"
        << "----------------\n"
        << "decode: "
        << (
            objects.exactSize &&
            objects.classicTypesOnly
                ? "EXACT"
                : "FAILED"
        )
        << " raw="
        << objectFile.bytes.size()
        << " stored="
        << objectFile.storedBytes.size()
        << " groups="
        << objects.objectIdGroups
        << " placements="
        << objects.placements
        << '\n';

    if (!objects.error.empty()) {
        std::cout
            << "note: "
            << objects.error
            << '\n';
    }

    const ObjectDeepResult deep =
        analyzeObjectsDeep(
            objects,
            locations
        );

    std::cout << "planes:";
    for (
        std::size_t plane = 0;
        plane < deep.planeCounts.size();
        ++plane
    ) {
        std::cout
            << " "
            << plane
            << "="
            << deep.planeCounts[plane];
    }
    std::cout << '\n';

    std::cout << "rotations:";
    for (
        std::size_t rotation = 0;
        rotation < deep.rotationCounts.size();
        ++rotation
    ) {
        std::cout
            << " "
            << rotation
            << "="
            << deep.rotationCounts[rotation];
    }
    std::cout << '\n';

    std::cout << "placement types:";
    for (
        std::size_t type = 0;
        type < deep.typeCounts.size();
        ++type
    ) {
        if (deep.typeCounts[type] == 0) {
            continue;
        }

        std::cout
            << " "
            << type
            << "="
            << deep.typeCounts[type];
    }
    std::cout << '\n';

    std::cout
        << "loc definitions: resolved="
        << deep.definitionResolved
        << " missing="
        << deep.definitionMissing
        << " no-model="
        << deep.definitionsWithNoModels
        << '\n'
        << "classic model-type hypothesis "
        << "(11->10, 5..8->4): compatible="
        << deep.modelTypeCompatible
        << " incompatible="
        << deep.modelTypeIncompatible
        << '\n';

    std::cout
        << "\ntop placed loc ids:\n";

    for (
        const auto& [objectId, count] :
        topMapCounts(
            deep.objectCounts,
            20
        )
    ) {
        const eld::definition::LocationDefinition* definition =
            objectId >= 0 && objectId <= 65535
                ? locations.find(
                      static_cast<std::uint16_t>(
                          objectId
                      )
                  )
                : nullptr;

        std::cout
            << "  loc="
            << objectId
            << " count="
            << count;

        if (definition == nullptr) {
            std::cout
                << " definition=MISSING\n";
            continue;
        }

        std::cout
            << " name=\""
            << definition->name
            << "\" size="
            << static_cast<unsigned int>(
                   definition->width
               )
            << "x"
            << static_cast<unsigned int>(
                   definition->length
               )
            << " models="
            << locationModelTypeSummary(
                   *definition
               )
            << '\n';
    }

    std::map<int, std::size_t> namedCounts;

    for (const auto& [objectId, count] : deep.objectCounts) {
        if (objectId < 0 || objectId > 65535) {
            continue;
        }

        const eld::definition::LocationDefinition* definition =
            locations.find(
                static_cast<std::uint16_t>(
                    objectId
                )
            );

        if (
            definition != nullptr &&
            !definition->name.empty() &&
            definition->name != "null"
        ) {
            namedCounts[objectId] = count;
        }
    }

    std::cout
        << "\ntop named locs:\n";

    for (
        const auto& [objectId, count] :
        topMapCounts(
            namedCounts,
            20
        )
    ) {
        const eld::definition::LocationDefinition* definition =
            locations.find(
                static_cast<std::uint16_t>(
                    objectId
                )
            );

        if (definition == nullptr) {
            continue;
        }

        std::cout
            << "  loc="
            << objectId
            << " count="
            << count
            << " name=\""
            << definition->name
            << "\" actions=";

        bool wroteAction = false;

        for (const std::string& action : definition->actions) {
            if (action.empty()) {
                continue;
            }

            if (wroteAction) {
                std::cout << '/';
            }

            std::cout << action;
            wroteAction = true;
        }

        if (!wroteAction) {
            std::cout << '-';
        }

        std::cout << '\n';
    }

    if (!deep.incompatibleSamples.empty()) {
        std::cout
            << "\nmodel-type incompatibility samples:\n";

        for (
            const ObjectPlacement& placement :
            deep.incompatibleSamples
        ) {
            const eld::definition::LocationDefinition* definition =
                placement.objectId >= 0 &&
                        placement.objectId <= 65535
                    ? locations.find(
                          static_cast<std::uint16_t>(
                              placement.objectId
                          )
                      )
                    : nullptr;

            std::cout
                << "  loc="
                << placement.objectId
                << " p="
                << placement.plane
                << " x="
                << placement.x
                << " y="
                << placement.y
                << " type="
                << placement.type
                << " normalized="
                << normalizedClassicLocationModelType(
                       placement.type
                   );

            if (definition != nullptr) {
                std::cout
                    << " name=\""
                    << definition->name
                    << "\" models="
                    << locationModelTypeSummary(
                           *definition
                       );
            }

            std::cout << '\n';
        }
    }
}

void validateAllSemanticLinks(
    const eld::cache::Cache& cache,
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex
) {
    const eld::definition::DefinitionRepository definitions(
        cache.open(eld::cache::IndexId::Config),
        2
    );
    const eld::definition::FloorRepository floors(
        definitions.get("flo")
    );
    const eld::definition::LocationRepository locations(
        definitions.get("loc")
    );

    std::size_t terrainExact = 0;
    std::size_t terrainFailures = 0;
    std::size_t objectExact = 0;
    std::size_t objectFailures = 0;

    FloorReferenceValidation overlayValidation;
    FloorReferenceValidation underlayValidation;

    std::size_t definitionResolved = 0;
    std::size_t definitionMissing = 0;
    std::size_t definitionsWithNoModels = 0;
    std::size_t modelTypeCompatible = 0;
    std::size_t modelTypeIncompatible = 0;

    std::vector<std::string> failureSamples;
    std::vector<std::string> incompatibleSamples;

    for (const MapIndexEntry& region : mapIndex) {
        try {
            const LoadedFile terrainFile =
                loadFile(
                    maps,
                    region.terrainFileId
                );
            const TerrainDeepResult terrain =
                decodeTerrainDeep(
                    terrainFile.bytes,
                    region.regionX(),
                    region.regionY()
                );

            if (terrain.exactSize) {
                ++terrainExact;
            }
            else {
                ++terrainFailures;
            }

            const FloorReferenceValidation overlays =
                validateFloorReferences(
                    terrain.overlayIds,
                    floors
                );
            const FloorReferenceValidation underlays =
                validateFloorReferences(
                    terrain.underlayIds,
                    floors
                );

            overlayValidation.valid += overlays.valid;
            overlayValidation.invalid += overlays.invalid;
            overlayValidation.zero += overlays.zero;
            underlayValidation.valid += underlays.valid;
            underlayValidation.invalid += underlays.invalid;
            underlayValidation.zero += underlays.zero;
        }
        catch (const std::exception& exception) {
            ++terrainFailures;

            if (failureSamples.size() < 12) {
                std::ostringstream stream;
                stream
                    << "terrain region="
                    << region.regionId
                    << " file="
                    << region.terrainFileId
                    << " "
                    << exception.what();
                failureSamples.push_back(
                    stream.str()
                );
            }
        }

        try {
            const LoadedFile objectFile =
                loadFile(
                    maps,
                    region.objectFileId
                );
            const ObjectProbeResult objects =
                probeObjects(
                    objectFile.bytes
                );

            if (
                objects.exactSize &&
                objects.classicTypesOnly
            ) {
                ++objectExact;
            }
            else {
                ++objectFailures;
            }

            const ObjectDeepResult deep =
                analyzeObjectsDeep(
                    objects,
                    locations
                );

            definitionResolved +=
                deep.definitionResolved;
            definitionMissing +=
                deep.definitionMissing;
            definitionsWithNoModels +=
                deep.definitionsWithNoModels;
            modelTypeCompatible +=
                deep.modelTypeCompatible;
            modelTypeIncompatible +=
                deep.modelTypeIncompatible;

            for (
                const ObjectPlacement& placement :
                deep.incompatibleSamples
            ) {
                if (incompatibleSamples.size() >= 20) {
                    break;
                }

                std::ostringstream stream;
                stream
                    << "region="
                    << region.regionId
                    << " loc="
                    << placement.objectId
                    << " p="
                    << placement.plane
                    << " x="
                    << placement.x
                    << " y="
                    << placement.y
                    << " type="
                    << placement.type
                    << " normalized="
                    << normalizedClassicLocationModelType(
                           placement.type
                       );

                if (
                    placement.objectId >= 0 &&
                    placement.objectId <= 65535
                ) {
                    const auto* definition =
                        locations.find(
                            static_cast<std::uint16_t>(
                                placement.objectId
                            )
                        );

                    if (definition != nullptr) {
                        stream
                            << " name=\""
                            << definition->name
                            << "\" models="
                            << locationModelTypeSummary(
                                   *definition
                               );
                    }
                }

                incompatibleSamples.push_back(
                    stream.str()
                );
            }
        }
        catch (const std::exception& exception) {
            ++objectFailures;

            if (failureSamples.size() < 12) {
                std::ostringstream stream;
                stream
                    << "objects region="
                    << region.regionId
                    << " file="
                    << region.objectFileId
                    << " "
                    << exception.what();
                failureSamples.push_back(
                    stream.str()
                );
            }
        }
    }

    const std::size_t modelChecked =
        modelTypeCompatible +
        modelTypeIncompatible;
    const double compatibilityPercent =
        modelChecked == 0
            ? 0.0
            : static_cast<double>(
                  modelTypeCompatible
              ) *
              100.0 /
              static_cast<double>(
                  modelChecked
              );

    std::cout
        << "SEMANTIC VALIDATION\n"
        << "===================\n"
        << "regions: "
        << mapIndex.size()
        << '\n'
        << "terrain exact: "
        << terrainExact
        << " failures: "
        << terrainFailures
        << '\n'
        << "object exact: "
        << objectExact
        << " failures: "
        << objectFailures
        << '\n'
        << "overlay floor refs: valid="
        << overlayValidation.valid
        << " invalid="
        << overlayValidation.invalid
        << " raw-zero="
        << overlayValidation.zero
        << '\n'
        << "underlay floor refs: valid="
        << underlayValidation.valid
        << " invalid="
        << underlayValidation.invalid
        << " raw-zero="
        << underlayValidation.zero
        << '\n'
        << "loc definitions: resolved="
        << definitionResolved
        << " missing="
        << definitionMissing
        << " no-model="
        << definitionsWithNoModels
        << '\n'
        << "model type hypothesis: compatible="
        << modelTypeCompatible
        << " incompatible="
        << modelTypeIncompatible
        << " rate="
        << std::fixed
        << std::setprecision(3)
        << compatibilityPercent
        << "%\n"
        << std::defaultfloat;

    if (!failureSamples.empty()) {
        std::cout
            << "\nload/decode failure samples:\n";

        for (const std::string& failure : failureSamples) {
            std::cout
                << "  "
                << failure
                << '\n';
        }
    }

    if (!incompatibleSamples.empty()) {
        std::cout
            << "\nmodel-type incompatibility samples:\n";

        for (const std::string& sample : incompatibleSamples) {
            std::cout
                << "  "
                << sample
                << '\n';
        }
    }
}

std::optional<unsigned long> parseUnsigned(
    std::string_view text
) {
    try {
        std::size_t consumed = 0;
        const unsigned long value =
            std::stoul(
                std::string(text),
                &consumed,
                0
            );

        if (consumed != text.size()) {
            return std::nullopt;
        }

        return value;
    }
    catch (...) {
        return std::nullopt;
    }
}

void printUsage(
    const char* executable
) {
    std::cerr
        << "usage:\n"
        << "  " << executable
        << " <cache-root> [summary]\n"
        << "  " << executable
        << " <cache-root> scan\n"
        << "  " << executable
        << " <cache-root> regions\n"
        << "  " << executable
        << " <cache-root> file <index4-file-id>\n"
        << "  " << executable
        << " <cache-root> region <region-id>\n"
        << "  " << executable
        << " <cache-root> region <region-x> <region-y>\n"
        << "  " << executable
        << " <cache-root> deep <region-id>\n"
        << "  " << executable
        << " <cache-root> deep <region-x> <region-y>\n"
        << "  " << executable
        << " <cache-root> validate\n"
        << "\nExamples:\n"
        << "  " << executable
        << " cache summary\n"
        << "  " << executable
        << " cache scan > map_scan.txt\n"
        << "  " << executable
        << " cache file 1234\n"
        << "  " << executable
        << " cache region 12850\n"
        << "  " << executable
        << " cache region 50 50\n"
        << "  " << executable
        << " cache deep 50 50\n"
        << "  " << executable
        << " cache validate\n";
}

} // namespace

int main(
    int argc,
    char** argv
) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        const std::filesystem::path cacheRoot =
            argv[1];

        const eld::cache::Cache cache(
            cacheRoot
        );

        const eld::cache::Store maps =
            cache.open(
                eld::cache::IndexId::Maps
            );

        const MapIndexLoadResult mapIndex =
            loadMapIndex(cache);

        const std::string command =
            argc >= 3
                ? argv[2]
                : "summary";

        if (command == "summary") {
            printSummary(
                cache,
                maps,
                mapIndex
            );
            return 0;
        }

        if (command == "regions") {
            printRegions(
                maps,
                mapIndex
            );
            return
                mapIndex.error.empty()
                    ? 0
                    : 2;
        }

        if (command == "scan") {
            if (!mapIndex.error.empty()) {
                std::cerr
                    << "warning: map_index unavailable: "
                    << mapIndex.error
                    << '\n';
            }

            scanAll(
                maps,
                mapIndex.entries
            );
            return 0;
        }

        if (command == "validate") {
            if (!mapIndex.error.empty()) {
                std::cerr
                    << "Cannot validate semantic links: "
                    << mapIndex.error
                    << '\n';
                return 2;
            }

            validateAllSemanticLinks(
                cache,
                maps,
                mapIndex.entries
            );
            return 0;
        }

        if (command == "file") {
            if (argc != 4) {
                printUsage(argv[0]);
                return 1;
            }

            const std::optional<unsigned long> fileId =
                parseUnsigned(argv[3]);

            if (
                !fileId.has_value() ||
                *fileId > 65535ul
            ) {
                std::cerr
                    << "invalid Index 4 file id: "
                    << argv[3]
                    << '\n';
                return 1;
            }

            printFileDetail(
                maps,
                mapIndex.entries,
                static_cast<std::uint16_t>(
                    *fileId
                )
            );
            return 0;
        }

        if (command == "deep") {
            if (
                argc != 4 &&
                argc != 5
            ) {
                printUsage(argv[0]);
                return 1;
            }

            if (!mapIndex.error.empty()) {
                std::cerr
                    << "Cannot deeply inspect a region: "
                    << mapIndex.error
                    << '\n';
                return 2;
            }

            std::uint16_t regionId = 0;

            if (argc == 4) {
                const std::optional<unsigned long> value =
                    parseUnsigned(argv[3]);

                if (
                    !value.has_value() ||
                    *value > 65535ul
                ) {
                    std::cerr
                        << "invalid region id: "
                        << argv[3]
                        << '\n';
                    return 1;
                }

                regionId =
                    static_cast<std::uint16_t>(
                        *value
                    );
            }
            else {
                const std::optional<unsigned long> x =
                    parseUnsigned(argv[3]);
                const std::optional<unsigned long> y =
                    parseUnsigned(argv[4]);

                if (
                    !x.has_value() ||
                    !y.has_value() ||
                    *x > 255ul ||
                    *y > 255ul
                ) {
                    std::cerr
                        << "region x/y must both be 0..255\n";
                    return 1;
                }

                regionId =
                    static_cast<std::uint16_t>(
                        (*x << 8) |
                        *y
                    );
            }

            printRegionDeep(
                cache,
                maps,
                mapIndex.entries,
                regionId
            );
            return 0;
        }

        if (command == "region") {
            if (
                argc != 4 &&
                argc != 5
            ) {
                printUsage(argv[0]);
                return 1;
            }

            if (!mapIndex.error.empty()) {
                std::cerr
                    << "Cannot inspect a region: "
                    << mapIndex.error
                    << '\n';
                return 2;
            }

            std::uint16_t regionId = 0;

            if (argc == 4) {
                const std::optional<unsigned long> value =
                    parseUnsigned(argv[3]);

                if (
                    !value.has_value() ||
                    *value > 65535ul
                ) {
                    std::cerr
                        << "invalid region id: "
                        << argv[3]
                        << '\n';
                    return 1;
                }

                regionId =
                    static_cast<std::uint16_t>(
                        *value
                    );
            }
            else {
                const std::optional<unsigned long> x =
                    parseUnsigned(argv[3]);
                const std::optional<unsigned long> y =
                    parseUnsigned(argv[4]);

                if (
                    !x.has_value() ||
                    !y.has_value() ||
                    *x > 255ul ||
                    *y > 255ul
                ) {
                    std::cerr
                        << "region x/y must both be 0..255\n";
                    return 1;
                }

                regionId =
                    static_cast<std::uint16_t>(
                        (*x << 8) |
                        *y
                    );
            }

            printRegionDetail(
                maps,
                mapIndex.entries,
                regionId
            );
            return 0;
        }

        std::cerr
            << "unknown command: "
            << command
            << '\n';
        printUsage(argv[0]);
        return 1;
    }
    catch (const std::exception& exception) {
        std::cerr
            << "map_probe failed: "
            << exception.what()
            << '\n';
        return 2;
    }
}
