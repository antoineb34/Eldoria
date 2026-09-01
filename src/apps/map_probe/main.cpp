#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
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
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "archive/ArchiveParser.h"
#include "binary/Compression.h"
#include "cache/Cache.h"
#include "cache/File.h"
#include "cache/Store.h"
#include "definition/DefinitionRepository.h"
#include "definition/floor/FloorRepository.h"
#include "definition/location/LocationRepository.h"
#include "map/ClassicTerrainAppearance.h"
#include "map/ClassicTileRules.h"
#include "map/MapLoader.h"
#include "map/SceneLocBuilder.h"
#include "map/SceneLocModelBuilder.h"
#include "map/SceneTerrainBuilder.h"
#include "GraphicsResources.h"
#include "model/ModelRepository.h"
#include "sdl/SdlContext.h"
#include "texture/TextureRepository.h"

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

struct TerrainTileVisual {
    int height = 0;
    std::optional<std::uint8_t> overlayId;
    std::optional<std::uint8_t> underlayId;
    std::optional<std::uint8_t> setting;
    std::optional<std::uint8_t> overlayShape;
    std::optional<std::uint8_t> overlayRotation;
};

struct TerrainDeepResult {
    bool exactSize = false;
    std::size_t consumed = 0;
    std::array<TerrainPlaneDeepStats, TerrainPlanes> planes{};
    std::array<TerrainTileVisual, TerrainTileCount> tiles{};
    std::array<std::size_t, 256> overlayIds{};
    std::array<std::size_t, 256> underlayIds{};
    std::array<std::size_t, 33> settingValues{};
    std::string error;
};

std::size_t terrainTileIndex(
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    return
        plane * RegionSize * RegionSize +
        x * RegionSize +
        y;
}

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
    std::int32_t n =
        static_cast<std::int32_t>(x + y * 57);
    n = static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(n) << 13) ^
        static_cast<std::uint32_t>(n)
    );

    const std::uint32_t un =
        static_cast<std::uint32_t>(n);
    const std::uint32_t hashed =
        (
            un *
            (un * un * 15731u + 789221u) +
            1376312589u
        ) &
        0x7fffffffu;

    return static_cast<int>((hashed >> 19) & 0xFFu);
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
    const int x = worldX + 932731;
    const int y = worldY + 556238;

    int height =
        classicInterpolatedNoise(
            x + 45365,
            y + 91923,
            4
        ) -
        128;

    height +=
        (
            classicInterpolatedNoise(
                x + 10294,
                y + 37821,
                2
            ) -
            128
        ) >>
        1;

    height +=
        (
            classicInterpolatedNoise(
                x,
                y,
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
                                    terrainTileIndex(
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
                                    terrainTileIndex(
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

                heights[terrainTileIndex(plane, x, y)] =
                    tileHeight;

                result.tiles[
                    terrainTileIndex(plane, x, y)
                ] = TerrainTileVisual{
                    .height = tileHeight,
                    .overlayId = overlayId,
                    .underlayId = underlayId,
                    .setting = setting,
                    .overlayShape = overlayShape,
                    .overlayRotation = overlayRotation
                };

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

struct ViewerColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

struct MapViewerState {
    std::size_t plane = 0;
    bool showUnderlays = true;
    bool showOverlays = true;
    bool showObjects = true;
    bool showHeightShading = true;
    bool showSettings = false;
    bool showGrid = false;
};

ViewerColor unpackRgb(
    std::uint32_t rgb
) {
    return ViewerColor{
        static_cast<std::uint8_t>((rgb >> 16) & 0xFFu),
        static_cast<std::uint8_t>((rgb >> 8) & 0xFFu),
        static_cast<std::uint8_t>(rgb & 0xFFu)
    };
}

ViewerColor fallbackFloorColor(
    std::uint8_t rawId
) {
    const std::uint32_t value =
        static_cast<std::uint32_t>(rawId) *
        2654435761u;

    return ViewerColor{
        static_cast<std::uint8_t>(64u + ((value >> 16) & 0x7Fu)),
        static_cast<std::uint8_t>(64u + ((value >> 8) & 0x7Fu)),
        static_cast<std::uint8_t>(64u + (value & 0x7Fu))
    };
}

ViewerColor floorColor(
    std::uint8_t rawId,
    const eld::definition::FloorRepository& floors
) {
    if (rawId == 0) {
        return ViewerColor{38, 42, 48};
    }

    const auto* definition =
        floors.find(
            static_cast<std::uint16_t>(rawId - 1u)
        );

    if (definition == nullptr) {
        return fallbackFloorColor(rawId);
    }

    if (definition->rgb.has_value()) {
        return unpackRgb(*definition->rgb);
    }

    if (definition->secondaryRgb.has_value()) {
        return unpackRgb(*definition->secondaryRgb);
    }

    return fallbackFloorColor(rawId);
}

ViewerColor shadeColor(
    ViewerColor color,
    int height,
    const TerrainPlaneDeepStats& stats,
    bool enabled
) {
    if (
        !enabled ||
        stats.minHeight >= stats.maxHeight
    ) {
        return color;
    }

    const double t =
        static_cast<double>(
            height - stats.minHeight
        ) /
        static_cast<double>(
            stats.maxHeight - stats.minHeight
        );

    const double factor =
        0.72 + t * 0.38;

    color.r = static_cast<std::uint8_t>(
        std::clamp(
            static_cast<int>(
                static_cast<double>(color.r) * factor
            ),
            0,
            255
        )
    );
    color.g = static_cast<std::uint8_t>(
        std::clamp(
            static_cast<int>(
                static_cast<double>(color.g) * factor
            ),
            0,
            255
        )
    );
    color.b = static_cast<std::uint8_t>(
        std::clamp(
            static_cast<int>(
                static_cast<double>(color.b) * factor
            ),
            0,
            255
        )
    );

    return color;
}

void setDrawColor(
    SDL_Renderer* renderer,
    ViewerColor color
) {
    SDL_SetRenderDrawColor(
        renderer,
        color.r,
        color.g,
        color.b,
        255
    );
}

std::string viewerTitle(
    const MapIndexEntry& region,
    const MapViewerState& state
) {
    std::ostringstream stream;
    stream
        << "Eldoria Map Probe | region "
        << region.regionId
        << " ("
        << region.regionX()
        << ","
        << region.regionY()
        << ") | plane "
        << state.plane
        << " | U underlay "
        << (state.showUnderlays ? "on" : "off")
        << " | O overlay "
        << (state.showOverlays ? "on" : "off")
        << " | L objects "
        << (state.showObjects ? "on" : "off")
        << " | H shade "
        << (state.showHeightShading ? "on" : "off")
        << " | S settings "
        << (state.showSettings ? "on" : "off")
        << " | G grid "
        << (state.showGrid ? "on" : "off");
    return stream.str();
}

void printViewerTile(
    const MapIndexEntry& region,
    const TerrainDeepResult& terrain,
    const ObjectProbeResult& objects,
    const eld::definition::LocationRepository& locations,
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    const TerrainTileVisual& tile =
        terrain.tiles[
            terrainTileIndex(plane, x, y)
        ];

    std::cout
        << "\nVIEW TILE"
        << " p=" << plane
        << " local=(" << x << "," << y << ")"
        << " world=("
        << region.regionX() * 64 + static_cast<int>(x)
        << ","
        << region.regionY() * 64 + static_cast<int>(y)
        << ")"
        << " height=" << tile.height;

    if (tile.underlayId.has_value()) {
        std::cout
            << " underlay="
            << static_cast<unsigned int>(*tile.underlayId);
    }

    if (tile.overlayId.has_value()) {
        std::cout
            << " overlay="
            << static_cast<unsigned int>(*tile.overlayId)
            << " shape="
            << static_cast<unsigned int>(
                   tile.overlayShape.value_or(0)
               )
            << " rot="
            << static_cast<unsigned int>(
                   tile.overlayRotation.value_or(0)
               );
    }

    if (tile.setting.has_value()) {
        std::cout
            << " setting="
            << static_cast<unsigned int>(*tile.setting);
    }

    std::cout << '\n';

    bool foundObject = false;

    for (const ObjectPlacement& placement : objects.decodedPlacements) {
        if (
            placement.plane != static_cast<int>(plane) ||
            placement.x != static_cast<int>(x) ||
            placement.y != static_cast<int>(y)
        ) {
            continue;
        }

        foundObject = true;

        const auto* definition =
            placement.objectId >= 0 &&
                    placement.objectId <= 65535
                ? locations.find(
                      static_cast<std::uint16_t>(
                          placement.objectId
                      )
                  )
                : nullptr;

        std::cout
            << "  loc=" << placement.objectId
            << " type=" << placement.type
            << " rotation=" << placement.rotation;

        if (definition != nullptr) {
            std::cout
                << " name=\""
                << definition->name
                << "\" size="
                << static_cast<unsigned int>(definition->width)
                << "x"
                << static_cast<unsigned int>(definition->length);
        }

        std::cout << '\n';
    }

    if (!foundObject) {
        std::cout << "  objects: none\n";
    }
}

void drawObjectMarker(
    SDL_Renderer* renderer,
    const ObjectPlacement& placement,
    float left,
    float top,
    float tileSize
) {
    const float right = left + tileSize;
    const float bottom = top + tileSize;

    if (placement.type >= 0 && placement.type <= 3) {
        SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);

        switch (placement.rotation & 3) {
            case 0:
                SDL_RenderLine(renderer, left, top, left, bottom);
                break;
            case 1:
                SDL_RenderLine(renderer, left, top, right, top);
                break;
            case 2:
                SDL_RenderLine(renderer, right, top, right, bottom);
                break;
            case 3:
                SDL_RenderLine(renderer, left, bottom, right, bottom);
                break;
        }

        return;
    }

    if (placement.type == 9) {
        SDL_SetRenderDrawColor(renderer, 255, 96, 220, 255);

        if ((placement.rotation & 1) == 0) {
            SDL_RenderLine(renderer, left, bottom, right, top);
        }
        else {
            SDL_RenderLine(renderer, left, top, right, bottom);
        }

        return;
    }

    ViewerColor color =
        placement.type == 22
            ? ViewerColor{255, 214, 64}
            : ViewerColor{245, 245, 245};

    setDrawColor(renderer, color);

    const float markerSize =
        std::max(2.0f, tileSize * 0.32f);
    const SDL_FRect marker{
        left + (tileSize - markerSize) * 0.5f,
        top + (tileSize - markerSize) * 0.5f,
        markerSize,
        markerSize
    };
    SDL_RenderFillRect(renderer, &marker);
}

void viewRegion(
    const eld::cache::Cache& cache,
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t regionId
) {
    const MapIndexEntry* region =
        findRegion(mapIndex, regionId);

    if (region == nullptr) {
        throw std::runtime_error(
            "map_index has no region " +
            std::to_string(regionId)
        );
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

    const LoadedFile terrainFile =
        loadFile(maps, region->terrainFileId);
    const TerrainDeepResult terrain =
        decodeTerrainDeep(
            terrainFile.bytes,
            region->regionX(),
            region->regionY()
        );

    if (!terrain.exactSize) {
        throw std::runtime_error(
            "terrain did not decode exactly: " +
            terrain.error
        );
    }

    const LoadedFile objectFile =
        loadFile(maps, region->objectFileId);
    const ObjectProbeResult objects =
        probeObjects(objectFile.bytes);

    if (
        !objects.exactSize ||
        !objects.classicTypesOnly
    ) {
        throw std::runtime_error(
            "objects did not decode exactly: " +
            objects.error
        );
    }

    std::cout
        << "MAP VISUAL PROBE\n"
        << "================\n"
        << "region: " << region->regionId
        << " (" << region->regionX()
        << "," << region->regionY() << ")\n"
        << "world base: x=" << region->regionX() * 64
        << " y=" << region->regionY() * 64 << '\n'
        << "terrain file: " << region->terrainFileId << '\n'
        << "object file: " << region->objectFileId << '\n'
        << "placements: " << objects.placements << "\n\n"
        << "Controls\n"
        << "--------\n"
        << "1 / 2 / 3 / 4 : plane 0 / 1 / 2 / 3\n"
        << "U             : toggle underlays\n"
        << "O             : toggle overlays\n"
        << "L             : toggle object markers\n"
        << "H             : toggle height shading\n"
        << "S             : toggle tile-setting markers\n"
        << "G             : toggle tile grid\n"
        << "Click tile    : dump tile + objects to terminal\n"
        << "Esc           : quit\n\n"
        << "Note: non-zero overlay shapes are shown as inset markers for now;\n"
        << "this viewer is a semantic diagnostic, not the final map renderer.\n";

    eld::platform::SdlContext sdl(
        "Eldoria Map Visual Probe",
        900,
        900
    );

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (window == nullptr || renderer == nullptr) {
        throw std::runtime_error(
            "failed to create SDL map viewer"
        );
    }

    MapViewerState state;
    bool running = true;
    bool titleDirty = true;

    while (running) {
        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        const float mapPixels =
            std::max(
                64.0f,
                static_cast<float>(
                    std::min(windowWidth, windowHeight)
                ) - 40.0f
            );
        const float tileSize =
            mapPixels /
            static_cast<float>(RegionSize);
        const float originX =
            (static_cast<float>(windowWidth) - mapPixels) * 0.5f;
        const float originY =
            (static_cast<float>(windowHeight) - mapPixels) * 0.5f;

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                const float localScreenX =
                    event.button.x - originX;
                const float localScreenY =
                    event.button.y - originY;

                if (
                    localScreenX >= 0.0f &&
                    localScreenY >= 0.0f &&
                    localScreenX < mapPixels &&
                    localScreenY < mapPixels
                ) {
                    const std::size_t x =
                        std::min(
                            RegionSize - 1,
                            static_cast<std::size_t>(
                                localScreenX / tileSize
                            )
                        );
                    const std::size_t screenY =
                        std::min(
                            RegionSize - 1,
                            static_cast<std::size_t>(
                                localScreenY / tileSize
                            )
                        );
                    const std::size_t y =
                        RegionSize - 1 - screenY;

                    printViewerTile(
                        *region,
                        terrain,
                        objects,
                        locations,
                        state.plane,
                        x,
                        y
                    );
                }

                continue;
            }

            if (event.type != SDL_EVENT_KEY_DOWN) {
                continue;
            }

            if (event.key.key == SDLK_ESCAPE) {
                running = false;
                continue;
            }

            switch (event.key.scancode) {
                case SDL_SCANCODE_1:
                    state.plane = 0;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_2:
                    state.plane = 1;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_3:
                    state.plane = 2;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_4:
                    state.plane = 3;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_U:
                    state.showUnderlays = !state.showUnderlays;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_O:
                    state.showOverlays = !state.showOverlays;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_L:
                    state.showObjects = !state.showObjects;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_H:
                    state.showHeightShading = !state.showHeightShading;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_S:
                    state.showSettings = !state.showSettings;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_G:
                    state.showGrid = !state.showGrid;
                    titleDirty = true;
                    break;
                default:
                    break;
            }
        }

        if (titleDirty) {
            const std::string title =
                viewerTitle(*region, state);
            SDL_SetWindowTitle(
                window,
                title.c_str()
            );
            titleDirty = false;
        }

        SDL_SetRenderDrawColor(
            renderer,
            18,
            20,
            24,
            255
        );
        SDL_RenderClear(renderer);

        const TerrainPlaneDeepStats& planeStats =
            terrain.planes[state.plane];

        for (std::size_t x = 0; x < RegionSize; ++x) {
            for (std::size_t y = 0; y < RegionSize; ++y) {
                const TerrainTileVisual& tile =
                    terrain.tiles[
                        terrainTileIndex(
                            state.plane,
                            x,
                            y
                        )
                    ];

                ViewerColor color{38, 42, 48};

                if (
                    state.showUnderlays &&
                    tile.underlayId.has_value()
                ) {
                    color =
                        floorColor(
                            *tile.underlayId,
                            floors
                        );
                }

                color =
                    shadeColor(
                        color,
                        tile.height,
                        planeStats,
                        state.showHeightShading
                    );

                const float left =
                    originX +
                    static_cast<float>(x) *
                    tileSize;
                const float top =
                    originY +
                    static_cast<float>(
                        RegionSize - 1 - y
                    ) *
                    tileSize;

                const SDL_FRect rect{
                    left,
                    top,
                    tileSize + 0.5f,
                    tileSize + 0.5f
                };

                setDrawColor(renderer, color);
                SDL_RenderFillRect(renderer, &rect);

                if (
                    state.showOverlays &&
                    tile.overlayId.has_value()
                ) {
                    ViewerColor overlay =
                        floorColor(
                            *tile.overlayId,
                            floors
                        );
                    overlay =
                        shadeColor(
                            overlay,
                            tile.height,
                            planeStats,
                            state.showHeightShading
                        );
                    setDrawColor(renderer, overlay);

                    if (
                        tile.overlayShape.value_or(0) == 0
                    ) {
                        SDL_RenderFillRect(renderer, &rect);
                    }
                    else {
                        const float inset =
                            tileSize * 0.22f;
                        const SDL_FRect overlayMarker{
                            left + inset,
                            top + inset,
                            std::max(1.0f, tileSize - inset * 2.0f),
                            std::max(1.0f, tileSize - inset * 2.0f)
                        };
                        SDL_RenderFillRect(
                            renderer,
                            &overlayMarker
                        );
                    }
                }

                if (
                    state.showSettings &&
                    tile.setting.has_value()
                ) {
                    SDL_SetRenderDrawColor(
                        renderer,
                        64,
                        255,
                        255,
                        255
                    );
                    SDL_RenderLine(
                        renderer,
                        left,
                        top,
                        left + tileSize,
                        top + tileSize
                    );
                }

                if (state.showGrid && tileSize >= 4.0f) {
                    SDL_SetRenderDrawColor(
                        renderer,
                        12,
                        14,
                        18,
                        255
                    );
                    SDL_RenderLine(
                        renderer,
                        left,
                        top,
                        left + tileSize,
                        top
                    );
                    SDL_RenderLine(
                        renderer,
                        left,
                        top,
                        left,
                        top + tileSize
                    );
                }
            }
        }

        if (state.showObjects) {
            for (
                const ObjectPlacement& placement :
                objects.decodedPlacements
            ) {
                if (
                    placement.plane !=
                        static_cast<int>(state.plane) ||
                    placement.x < 0 ||
                    placement.x >= static_cast<int>(RegionSize) ||
                    placement.y < 0 ||
                    placement.y >= static_cast<int>(RegionSize)
                ) {
                    continue;
                }

                const float left =
                    originX +
                    static_cast<float>(placement.x) *
                    tileSize;
                const float top =
                    originY +
                    static_cast<float>(
                        RegionSize - 1 -
                        static_cast<std::size_t>(placement.y)
                    ) *
                    tileSize;

                drawObjectMarker(
                    renderer,
                    placement,
                    left,
                    top,
                    tileSize
                );
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(8);
    }
}


struct Map3dViewerState {
    std::size_t plane = 0;
    float yaw = 0.75f;
    float pitch = 0.62f;
    float distance = 82.0f;
    bool showUnderlays = true;
    bool showOverlays = true;
    bool showGrid = false;
    bool showLocs = false;
    bool showLocModels = true;
    bool showDebugPanel = true;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ProjectedPoint {
    float x = 0.0f;
    float y = 0.0f;
    float depth = 0.0f;
    bool visible = false;
};

struct ProjectionContext {
    Vec3 camera{};
    Vec3 right{};
    Vec3 up{};
    Vec3 forward{};
    int windowWidth = 0;
    int windowHeight = 0;
    float focalLength = 1.0f;
};

struct ProjectedTriangle {
    std::array<SDL_Vertex, 3> vertices{};
    SDL_Texture* texture = nullptr;
    float depth = 0.0f;
};

struct CachedTerrainTriangle {
    std::array<std::uint8_t, 3> indices{};
    std::array<SDL_FColor, 3> colors{};
    std::array<SDL_FPoint, 3> uvs{};
    SDL_Texture* texture = nullptr;
    eld::graphics::map::TerrainSurface surface =
        eld::graphics::map::TerrainSurface::Underlay;
};

struct CachedTerrainTile {
    std::vector<Vec3> vertices;
    std::vector<CachedTerrainTriangle> triangles;
};

struct CachedTerrainPlane {
    std::vector<CachedTerrainTile> tiles;
    std::size_t sourceVertices = 0;
    std::size_t triangles = 0;
};

struct CachedLocMesh {
    const eld::graphics::RenderModel* model = nullptr;
    const eld::graphics::RenderMesh* mesh = nullptr;
    std::vector<Vec3> worldVertices;
    std::vector<Vec3> alternateWorldVertices;
};

struct CachedLocPart {
    bool cameraDependent = false;
    std::vector<CachedLocMesh> meshes;
};

struct CachedLocInstance {
    std::uint8_t scenePlane = 0;
    std::uint8_t rotation = 0;
    int sceneX = 0;
    int sceneZ = 0;
    Vec3 boundsCenter{};
    float boundsRadius = 0.0f;
    std::vector<CachedLocPart> parts;
};

struct CachedLocScene {
    std::vector<CachedLocInstance> instances;
    std::size_t parts = 0;
    std::size_t meshes = 0;
    std::size_t worldVertices = 0;
    std::size_t alternateWorldVertices = 0;
};


struct Map3dFrameTimings {
    double inputMs = 0.0;
    double cameraMs = 0.0;
    double terrainMs = 0.0;
    double locModelsMs = 0.0;
    double sortMs = 0.0;
    double submitMs = 0.0;
    double debugDrawMs = 0.0;
    double panelMs = 0.0;
    double presentMs = 0.0;
    double cpuFrameMs = 0.0;
};

struct Map3dFrameCounters {
    std::size_t terrainTiles = 0;
    std::size_t terrainVerticesProjected = 0;
    std::size_t terrainTriangles = 0;
    std::size_t terrainTexturedTriangles = 0;

    std::size_t locInstances = 0;
    std::size_t locInstancesCulled = 0;
    std::size_t locInstancesRendered = 0;
    std::size_t locParts = 0;
    std::size_t locMeshes = 0;
    std::size_t locVerticesProjected = 0;
    std::size_t locTriangleCandidates = 0;
    std::size_t locTriangles = 0;
    std::size_t locTexturedTriangles = 0;

    std::size_t totalTriangles = 0;
    std::size_t geometryVertices = 0;
    std::size_t geometryBatches = 0;
};

void addFrameTimings(
    Map3dFrameTimings& sum,
    const Map3dFrameTimings& value
) {
    sum.inputMs += value.inputMs;
    sum.cameraMs += value.cameraMs;
    sum.terrainMs += value.terrainMs;
    sum.locModelsMs += value.locModelsMs;
    sum.sortMs += value.sortMs;
    sum.submitMs += value.submitMs;
    sum.debugDrawMs += value.debugDrawMs;
    sum.panelMs += value.panelMs;
    sum.presentMs += value.presentMs;
    sum.cpuFrameMs += value.cpuFrameMs;
}

Map3dFrameTimings averageFrameTimings(
    const Map3dFrameTimings& sum,
    std::size_t frames
) {
    if (frames == 0) {
        return {};
    }

    const double divisor = static_cast<double>(frames);
    return Map3dFrameTimings{
        .inputMs = sum.inputMs / divisor,
        .cameraMs = sum.cameraMs / divisor,
        .terrainMs = sum.terrainMs / divisor,
        .locModelsMs = sum.locModelsMs / divisor,
        .sortMs = sum.sortMs / divisor,
        .submitMs = sum.submitMs / divisor,
        .debugDrawMs = sum.debugDrawMs / divisor,
        .panelMs = sum.panelMs / divisor,
        .presentMs = sum.presentMs / divisor,
        .cpuFrameMs = sum.cpuFrameMs / divisor
    };
}

constexpr float DebugPanelX = 12.0f;
constexpr float DebugPanelY = 12.0f;
constexpr float DebugPanelWidth = 370.0f;
constexpr float DebugPanelHeight = 274.0f;

SDL_FRect map3dDebugDumpButtonRect() {
    return SDL_FRect{
        DebugPanelX + 12.0f,
        DebugPanelY + DebugPanelHeight - 44.0f,
        DebugPanelWidth - 24.0f,
        26.0f
    };
}

bool pointInsideRect(
    float x,
    float y,
    const SDL_FRect& rect
) {
    return
        x >= rect.x &&
        y >= rect.y &&
        x < rect.x + rect.w &&
        y < rect.y + rect.h;
}

void drawMap3dDebugText(
    SDL_Renderer* renderer,
    float x,
    float y,
    const std::string& text,
    std::uint8_t red = 230,
    std::uint8_t green = 235,
    std::uint8_t blue = 240
) {
    SDL_SetRenderDrawColor(
        renderer,
        red,
        green,
        blue,
        255
    );
    SDL_RenderDebugText(
        renderer,
        x,
        y,
        text.c_str()
    );
}

std::string formatDebugMetric(
    std::string_view label,
    double value,
    std::string_view suffix = " MS"
) {
    std::ostringstream stream;
    stream
        << label
        << ' '
        << std::fixed
        << std::setprecision(2)
        << value
        << suffix;
    return stream.str();
}

void drawMap3dDebugPanel(
    SDL_Renderer* renderer,
    const Map3dViewerState& state,
    double fps,
    double frameTimeMs,
    const Map3dFrameTimings& average,
    const Map3dFrameCounters& counters,
    const std::string& dumpStatus
) {
    const SDL_FRect panel{
        DebugPanelX,
        DebugPanelY,
        DebugPanelWidth,
        DebugPanelHeight
    };

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );
    SDL_SetRenderDrawColor(
        renderer,
        8,
        10,
        14,
        220
    );
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(
        renderer,
        92,
        105,
        120,
        255
    );
    SDL_RenderRect(renderer, &panel);

    float x = DebugPanelX + 12.0f;
    float y = DebugPanelY + 10.0f;
    constexpr float Line = 14.0f;

    drawMap3dDebugText(
        renderer,
        x,
        y,
        "MAP PROBE PERF  [F3 HIDE]"
    );
    y += Line;

    {
        std::ostringstream line;
        line
            << "FPS "
            << std::fixed
            << std::setprecision(1)
            << fps
            << "   FRAME "
            << frameTimeMs
            << " MS";
        drawMap3dDebugText(renderer, x, y, line.str(), 120, 240, 160);
    }
    y += Line;

    drawMap3dDebugText(
        renderer,
        x,
        y,
        formatDebugMetric("CPU", average.cpuFrameMs)
    );
    y += Line;

    {
        std::ostringstream line;
        line
            << formatDebugMetric("TERRAIN", average.terrainMs)
            << "   TRIS "
            << counters.terrainTriangles;
        drawMap3dDebugText(renderer, x, y, line.str());
    }
    y += Line;

    {
        std::ostringstream line;
        line
            << formatDebugMetric("LOC MODELS", average.locModelsMs)
            << "   TRIS "
            << counters.locTriangles;
        drawMap3dDebugText(renderer, x, y, line.str());
    }
    y += Line;

    drawMap3dDebugText(
        renderer,
        x,
        y,
        formatDebugMetric("SORT", average.sortMs)
    );
    y += Line;

    {
        std::ostringstream line;
        line
            << formatDebugMetric("SUBMIT", average.submitMs)
            << "   BATCHES "
            << counters.geometryBatches;
        drawMap3dDebugText(renderer, x, y, line.str());
    }
    y += Line;

    drawMap3dDebugText(
        renderer,
        x,
        y,
        formatDebugMetric("DEBUG DRAW", average.debugDrawMs)
    );
    y += Line;

    drawMap3dDebugText(
        renderer,
        x,
        y,
        formatDebugMetric("PRESENT", average.presentMs)
    );
    y += Line;

    {
        std::ostringstream line;
        line
            << "TOTAL TRIS "
            << counters.totalTriangles
            << "   GEOM VERTS "
            << counters.geometryVertices;
        drawMap3dDebugText(renderer, x, y, line.str());
    }
    y += Line;

    {
        std::ostringstream line;
        line
            << "LOC INST "
            << counters.locInstancesRendered
            << "/"
            << counters.locInstances
            << "   CULLED "
            << counters.locInstancesCulled
            << "   PROJ VERTS "
            << counters.locVerticesProjected;
        drawMap3dDebugText(renderer, x, y, line.str());
    }
    y += Line;

    {
        std::ostringstream line;
        line
            << "PLANE "
            << state.plane
            << "   MODELS "
            << (state.showLocModels ? "ON" : "OFF")
            << "   GRID "
            << (state.showGrid ? "ON" : "OFF");
        drawMap3dDebugText(renderer, x, y, line.str());
    }

    const SDL_FRect button = map3dDebugDumpButtonRect();
    SDL_SetRenderDrawColor(
        renderer,
        42,
        52,
        66,
        245
    );
    SDL_RenderFillRect(renderer, &button);
    SDL_SetRenderDrawColor(
        renderer,
        110,
        130,
        150,
        255
    );
    SDL_RenderRect(renderer, &button);
    drawMap3dDebugText(
        renderer,
        button.x + 12.0f,
        button.y + 9.0f,
        "DUMP DEBUG REPORT  [F9]",
        245,
        245,
        245
    );

    if (!dumpStatus.empty()) {
        drawMap3dDebugText(
            renderer,
            DebugPanelX + 12.0f,
            DebugPanelY + DebugPanelHeight - 12.0f,
            dumpStatus,
            135,
            220,
            255
        );
    }
}

class Map3dTextureCache {
public:
    Map3dTextureCache(
        SDL_Renderer* renderer,
        const eld::texture::TextureRepository& repository
    )
        : renderer_(renderer),
          repository_(repository) {
    }

    ~Map3dTextureCache() {
        for (const auto& [id, texture] : textures_) {
            (void)id;
            if (texture != nullptr) {
                SDL_DestroyTexture(texture);
            }
        }
    }

    Map3dTextureCache(const Map3dTextureCache&) = delete;
    Map3dTextureCache& operator=(const Map3dTextureCache&) = delete;

    SDL_Texture* get(std::uint16_t sourceTextureId) {
        const auto existing = textures_.find(sourceTextureId);
        if (existing != textures_.end()) {
            return existing->second;
        }

        const eld::image::Image image =
            repository_.getImage(sourceTextureId);

        if (
            image.width == 0 ||
            image.height == 0 ||
            image.pixels.size() !=
                static_cast<std::size_t>(image.width) *
                static_cast<std::size_t>(image.height)
        ) {
            throw std::runtime_error(
                "decoded terrain texture has invalid dimensions: " +
                std::to_string(sourceTextureId)
            );
        }

        static_assert(
            sizeof(eld::image::RgbaPixel) == 4,
            "terrain texture upload expects tightly packed RGBA pixels"
        );

        SDL_Texture* texture = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            static_cast<int>(image.width),
            static_cast<int>(image.height)
        );

        if (texture == nullptr) {
            throw std::runtime_error(
                "SDL_CreateTexture failed for terrain texture " +
                std::to_string(sourceTextureId) +
                ": " + SDL_GetError()
            );
        }

        const auto failTexture = [&](const char* operation) {
            const std::string error = SDL_GetError();
            SDL_DestroyTexture(texture);
            throw std::runtime_error(
                std::string(operation) +
                " failed for terrain texture " +
                std::to_string(sourceTextureId) +
                ": " + error
            );
        };

        if (!SDL_UpdateTexture(
                texture,
                nullptr,
                image.pixels.data(),
                static_cast<int>(
                    static_cast<std::size_t>(image.width) *
                    sizeof(eld::image::RgbaPixel)
                )
            )) {
            failTexture("SDL_UpdateTexture");
        }

        if (!SDL_SetTextureScaleMode(
                texture,
                SDL_SCALEMODE_NEAREST
            )) {
            failTexture("SDL_SetTextureScaleMode");
        }

        if (!SDL_SetTextureBlendMode(
                texture,
                SDL_BLENDMODE_BLEND
            )) {
            failTexture("SDL_SetTextureBlendMode");
        }

        textures_.emplace(sourceTextureId, texture);
        return texture;
    }

    std::size_t count() const {
        return textures_.size();
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    const eld::texture::TextureRepository& repository_;
    std::map<std::uint16_t, SDL_Texture*> textures_;
};

class Map3dGraphicsTextureCache {
public:
    Map3dGraphicsTextureCache(
        SDL_Renderer* renderer,
        const eld::graphics::GraphicsResources& resources
    )
        : renderer_(renderer),
          resources_(resources) {
    }

    ~Map3dGraphicsTextureCache() {
        for (const auto& [id, texture] : textures_) {
            (void)id;
            if (texture != nullptr) {
                SDL_DestroyTexture(texture);
            }
        }
    }

    Map3dGraphicsTextureCache(
        const Map3dGraphicsTextureCache&
    ) = delete;
    Map3dGraphicsTextureCache& operator=(
        const Map3dGraphicsTextureCache&
    ) = delete;

    SDL_Texture* get(eld::graphics::TextureHandle handle) {
        const auto existing = textures_.find(handle.value);
        if (existing != textures_.end()) {
            return existing->second;
        }

        const eld::graphics::GraphicsTexture& textureData =
            resources_.getTexture(handle);

        if (
            textureData.width == 0 ||
            textureData.height == 0 ||
            textureData.pixels.size() !=
                static_cast<std::size_t>(textureData.width) *
                static_cast<std::size_t>(textureData.height) * 4U
        ) {
            throw std::runtime_error(
                "graphics texture has invalid RGBA dimensions"
            );
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            static_cast<int>(textureData.width),
            static_cast<int>(textureData.height)
        );

        if (texture == nullptr) {
            throw std::runtime_error(
                std::string("SDL_CreateTexture failed for loc texture: ") +
                SDL_GetError()
            );
        }

        const auto failTexture = [&](const char* operation) {
            const std::string error = SDL_GetError();
            SDL_DestroyTexture(texture);
            throw std::runtime_error(
                std::string(operation) +
                " failed for loc texture: " + error
            );
        };

        if (!SDL_UpdateTexture(
                texture,
                nullptr,
                textureData.pixels.data(),
                static_cast<int>(
                    static_cast<std::size_t>(textureData.width) * 4U
                )
            )) {
            failTexture("SDL_UpdateTexture");
        }

        if (!SDL_SetTextureScaleMode(
                texture,
                SDL_SCALEMODE_NEAREST
            )) {
            failTexture("SDL_SetTextureScaleMode");
        }

        if (!SDL_SetTextureBlendMode(
                texture,
                SDL_BLENDMODE_BLEND
            )) {
            failTexture("SDL_SetTextureBlendMode");
        }

        textures_.emplace(handle.value, texture);
        return texture;
    }

    std::size_t count() const {
        return textures_.size();
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    const eld::graphics::GraphicsResources& resources_;
    std::map<std::uint32_t, SDL_Texture*> textures_;
};

Vec3 subtractVec3(
    Vec3 a,
    Vec3 b
) {
    return Vec3{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

float dotVec3(
    Vec3 a,
    Vec3 b
) {
    return
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

Vec3 crossVec3(
    Vec3 a,
    Vec3 b
) {
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalizeVec3(
    Vec3 value
) {
    const float length =
        std::sqrt(dotVec3(value, value));

    if (length <= 0.00001f) {
        return Vec3{};
    }

    return Vec3{
        value.x / length,
        value.y / length,
        value.z / length
    };
}

float terrainWorldHeight(
    int height
) {
    // The decoded map heights are in the classic 1/128-tile scale.
    // They become more negative as the terrain rises, so invert them.
    return
        -static_cast<float>(height) /
        128.0f;
}

Vec3 terrainPoint(
    std::size_t x,
    std::size_t y,
    int height
) {
    constexpr float Center =
        static_cast<float>(RegionSize - 1) * 0.5f;

    return Vec3{
        static_cast<float>(x) - Center,
        terrainWorldHeight(height),
        static_cast<float>(y) - Center
    };
}

Vec3 sceneTerrainPoint(
    const eld::graphics::map::SceneTerrainVertex& vertex
) {
    constexpr float TileUnits =
        static_cast<float>(
            eld::graphics::map::SceneTerrainBuilder::TileSize
        );
    constexpr float Center =
        static_cast<float>(RegionSize) * 0.5f;

    return Vec3{
        static_cast<float>(vertex.x) / TileUnits - Center,
        terrainWorldHeight(vertex.y),
        static_cast<float>(vertex.z) / TileUnits - Center
    };
}

ViewerColor terrain3dSurfaceColor(
    const TerrainTileVisual& tile,
    eld::graphics::map::TerrainSurface surface,
    const eld::definition::FloorRepository& floors
) {
    if (
        surface == eld::graphics::map::TerrainSurface::Overlay &&
        tile.overlayId.has_value()
    ) {
        return floorColor(
            *tile.overlayId,
            floors
        );
    }

    if (
        surface == eld::graphics::map::TerrainSurface::Underlay &&
        tile.underlayId.has_value()
    ) {
        return floorColor(
            *tile.underlayId,
            floors
        );
    }

    return
        surface == eld::graphics::map::TerrainSurface::Overlay
            ? ViewerColor{96, 76, 116}
            : ViewerColor{54, 60, 66};
}

SDL_FColor toSdlColor(
    ViewerColor color
) {
    constexpr float Scale = 1.0f / 255.0f;

    return SDL_FColor{
        static_cast<float>(color.r) * Scale,
        static_cast<float>(color.g) * Scale,
        static_cast<float>(color.b) * Scale,
        1.0f
    };
}

SDL_FColor toSdlColorRgb(
    std::uint32_t rgb
) {
    return toSdlColor(unpackRgb(rgb));
}

ProjectionContext makeProjectionContext(
    Vec3 camera,
    Vec3 right,
    Vec3 up,
    Vec3 forward,
    int windowWidth,
    int windowHeight
) {
    constexpr float FovRadians = 0.96f;
    const float halfViewport =
        static_cast<float>(
            std::min(windowWidth, windowHeight)
        ) * 0.5f;

    return ProjectionContext{
        .camera = camera,
        .right = right,
        .up = up,
        .forward = forward,
        .windowWidth = windowWidth,
        .windowHeight = windowHeight,
        .focalLength =
            halfViewport / std::tan(FovRadians * 0.5f)
    };
}

ProjectedPoint projectPoint(
    Vec3 point,
    const ProjectionContext& projection
) {
    const Vec3 relative =
        subtractVec3(point, projection.camera);

    const float cameraX =
        dotVec3(relative, projection.right);
    const float cameraY =
        dotVec3(relative, projection.up);
    const float cameraZ =
        dotVec3(relative, projection.forward);

    if (cameraZ <= 0.5f) {
        return ProjectedPoint{};
    }

    return ProjectedPoint{
        static_cast<float>(projection.windowWidth) * 0.5f +
            cameraX * projection.focalLength / cameraZ,
        static_cast<float>(projection.windowHeight) * 0.5f -
            cameraY * projection.focalLength / cameraZ,
        cameraZ,
        true
    };
}

ProjectedPoint projectPoint(
    Vec3 point,
    Vec3 camera,
    Vec3 right,
    Vec3 up,
    Vec3 forward,
    int windowWidth,
    int windowHeight
) {
    return projectPoint(
        point,
        makeProjectionContext(
            camera,
            right,
            up,
            forward,
            windowWidth,
            windowHeight
        )
    );
}

bool sphereVisible(
    Vec3 center,
    float radius,
    const ProjectionContext& projection
) {
    const Vec3 relative =
        subtractVec3(center, projection.camera);
    const float cameraX = dotVec3(relative, projection.right);
    const float cameraY = dotVec3(relative, projection.up);
    const float cameraZ = dotVec3(relative, projection.forward);

    if (cameraZ + radius <= 0.5f) {
        return false;
    }

    const float depth = std::max(cameraZ, 0.5f);
    const float horizontalExtent =
        depth *
            (static_cast<float>(projection.windowWidth) * 0.5f) /
            projection.focalLength +
        radius;
    const float verticalExtent =
        depth *
            (static_cast<float>(projection.windowHeight) * 0.5f) /
            projection.focalLength +
        radius;

    return
        std::abs(cameraX) <= horizontalExtent &&
        std::abs(cameraY) <= verticalExtent;
}


Vec3 sceneLocPoint(
    int sceneX,
    int sceneY,
    int sceneZ
) {
    constexpr float TileUnits =
        static_cast<float>(
            eld::graphics::map::SceneLocBuilder::TileSize
        );
    constexpr float Center =
        static_cast<float>(RegionSize) * 0.5f;

    return Vec3{
        static_cast<float>(sceneX) / TileUnits - Center,
        terrainWorldHeight(sceneY),
        static_cast<float>(sceneZ) / TileUnits - Center
    };
}

Vec3 rotateSceneYaw(
    Vec3 local,
    int yaw
) {
    constexpr float TwoPi = 6.28318530717958647692f;
    const float angle =
        static_cast<float>(yaw & 0x7FF) *
        TwoPi / 2048.0f;
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);

    return Vec3{
        local.x * cosine + local.z * sine,
        local.y,
        local.z * cosine - local.x * sine
    };
}

float sceneLocContourDelta(
    const eld::graphics::map::SceneLocModelInstance& instance,
    float modelX,
    float modelZ
) {
    const int southwest = instance.cornerHeights[0];
    const int southeast = instance.cornerHeights[1];
    const int northeast = instance.cornerHeights[2];
    const int northwest = instance.cornerHeights[3];

    const int x = static_cast<int>(modelX);
    const int z = static_cast<int>(modelZ);

    const int south =
        southwest +
        (southeast - southwest) * (x + 64) / 128;
    const int north =
        northwest +
        (northeast - northwest) * (x + 64) / 128;
    const int height =
        south + (north - south) * (z + 64) / 128;

    // LocType uses integer / 4 here (truncate toward zero), while World3D's
    // placement anchor is built with >> 2. Preserve that tiny historical
    // distinction instead of silently reusing sceneY.
    const int groundY =
        (southwest + southeast + northeast + northwest) / 4;

    return static_cast<float>(height - groundY);
}

struct SceneLocDrawTransform {
    int sceneX = 0;
    int sceneZ = 0;
    int yaw = 0;
};

bool sceneLocDiagonalUsesInset(
    int rotation,
    int sceneX,
    int sceneZ,
    Vec3 camera
) {
    constexpr float CenterTiles =
        static_cast<float>(RegionSize) * 0.5f;

    const float eyeSceneX =
        (camera.x + CenterTiles) * 128.0f;
    const float eyeSceneZ =
        (camera.z + CenterTiles) * 128.0f;
    const float relativeX =
        static_cast<float>(sceneX) - eyeSceneX;
    const float relativeZ =
        static_cast<float>(sceneZ) - eyeSceneZ;

    const int cardinal = rotation & 3;
    const float nearestX =
        cardinal == 1 || cardinal == 2
            ? -relativeX
            : relativeX;
    const float nearestZ =
        cardinal == 2 || cardinal == 3
            ? -relativeZ
            : relativeZ;

    // World3D's front/back passes select one of the two diagonal faces
    // according to the camera side. Equality is rare; prefer inset.
    return nearestZ <= nearestX;
}

SceneLocDrawTransform sceneLocFixedDrawTransform(
    const eld::graphics::map::SceneLocModelInstance& instance,
    const eld::graphics::map::SceneLocModelPart& part,
    bool diagonalInset
) {
    using eld::graphics::map::SceneLocDrawMode;

    SceneLocDrawTransform result{
        instance.sceneX,
        instance.sceneZ,
        part.sceneYaw
    };

    if (part.drawMode == SceneLocDrawMode::Standard) {
        return result;
    }

    constexpr std::array<int, 4> InsetX{53, -53, -53, 53};
    constexpr std::array<int, 4> InsetZ{-53, -53, 53, 53};
    constexpr std::array<int, 4> OutsetX{-45, 45, 45, -45};
    constexpr std::array<int, 4> OutsetZ{45, 45, -45, -45};

    const int rotation = instance.rotation & 3;
    bool useInset =
        part.drawMode == SceneLocDrawMode::WallDecorationInset;

    if (part.drawMode == SceneLocDrawMode::WallDecorationDiagonalBoth) {
        useInset = diagonalInset;
    }

    if (useInset) {
        result.sceneX += InsetX[rotation];
        result.sceneZ += InsetZ[rotation];
        result.yaw = rotation * 512 + 256;
    }
    else {
        result.sceneX += OutsetX[rotation];
        result.sceneZ += OutsetZ[rotation];
        result.yaw = (rotation * 512 + 1280) & 0x7FF;
    }

    return result;
}

SceneLocDrawTransform sceneLocDrawTransform(
    const eld::graphics::map::SceneLocModelInstance& instance,
    const eld::graphics::map::SceneLocModelPart& part,
    Vec3 camera
) {
    using eld::graphics::map::SceneLocDrawMode;

    const bool diagonalInset =
        part.drawMode != SceneLocDrawMode::WallDecorationDiagonalBoth ||
        sceneLocDiagonalUsesInset(
            instance.rotation,
            instance.sceneX,
            instance.sceneZ,
            camera
        );

    return sceneLocFixedDrawTransform(
        instance,
        part,
        diagonalInset
    );
}

Vec3 sceneLocModelPoint(
    const eld::graphics::map::SceneLocModelInstance& instance,
    const eld::graphics::RenderVertex& vertex,
    const SceneLocDrawTransform& draw
) {
    const Vec3 anchor =
        sceneLocPoint(draw.sceneX, instance.sceneY, draw.sceneZ);

    Vec3 local{
        vertex.position.x / 128.0f,
        vertex.position.y / 128.0f,
        vertex.position.z / 128.0f
    };

    if (instance.contouredGround) {
        // RenderModelAssembler converts source Y to graphics -Y. LocType's
        // hillskew adds the terrain delta to source Y, so subtract it here.
        const float delta = sceneLocContourDelta(
            instance,
            vertex.position.x,
            vertex.position.z
        );
        local.y -= delta / 128.0f;
    }

    local = rotateSceneYaw(local, draw.yaw);

    return Vec3{
        anchor.x + local.x,
        anchor.y + local.y,
        anchor.z + local.z
    };
}

Vec3 sceneLocModelPoint(
    const eld::graphics::map::SceneLocModelInstance& instance,
    const eld::graphics::map::SceneLocModelPart& part,
    const eld::graphics::RenderVertex& vertex,
    Vec3 camera
) {
    return sceneLocModelPoint(
        instance,
        vertex,
        sceneLocDrawTransform(instance, part, camera)
    );
}

float addressTextureCoordinate(
    float value,
    eld::graphics::TextureAddressMode mode
) {
    if (mode == eld::graphics::TextureAddressMode::Clamp) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    const float wrapped = value - std::floor(value);
    return wrapped < 0.0f ? wrapped + 1.0f : wrapped;
}

SDL_FPoint renderVertexUv(
    const eld::graphics::RenderVertex& vertex,
    const eld::graphics::RenderMaterial& material
) {
    return SDL_FPoint{
        addressTextureCoordinate(
            vertex.uv.x,
            material.sampler.addressU
        ),
        addressTextureCoordinate(
            vertex.uv.y,
            material.sampler.addressV
        )
    };
}

SDL_FColor renderVertexColor(
    const eld::graphics::RenderVertex& vertex,
    const eld::graphics::RenderMaterial& material
) {
    return SDL_FColor{
        std::clamp(
            vertex.color.x * material.baseColor.x,
            0.0f,
            1.0f
        ),
        std::clamp(
            vertex.color.y * material.baseColor.y,
            0.0f,
            1.0f
        ),
        std::clamp(
            vertex.color.z * material.baseColor.z,
            0.0f,
            1.0f
        ),
        std::clamp(
            vertex.color.w * material.baseColor.w,
            0.0f,
            1.0f
        )
    };
}

CachedLocScene buildCachedLocScene(
    const eld::graphics::map::SceneLocModelBuildResult& locModels,
    const std::vector<eld::graphics::ModelHandle>& variantHandles,
    const eld::graphics::GraphicsResources& resources
) {
    using eld::graphics::map::SceneLocDrawMode;

    CachedLocScene cache;
    cache.instances.reserve(locModels.instances.size());

    for (
        const eld::graphics::map::SceneLocModelInstance& instance :
        locModels.instances
    ) {
        CachedLocInstance cachedInstance;
        cachedInstance.scenePlane = instance.scenePlane;
        cachedInstance.rotation = instance.rotation;
        cachedInstance.sceneX = instance.sceneX;
        cachedInstance.sceneZ = instance.sceneZ;
        cachedInstance.parts.reserve(instance.parts.size());

        Vec3 boundsMin{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        };
        Vec3 boundsMax{
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()
        };
        bool haveBounds = false;

        auto includeBounds = [&](Vec3 point) {
            haveBounds = true;
            boundsMin.x = std::min(boundsMin.x, point.x);
            boundsMin.y = std::min(boundsMin.y, point.y);
            boundsMin.z = std::min(boundsMin.z, point.z);
            boundsMax.x = std::max(boundsMax.x, point.x);
            boundsMax.y = std::max(boundsMax.y, point.y);
            boundsMax.z = std::max(boundsMax.z, point.z);
        };

        for (
            const eld::graphics::map::SceneLocModelPart& part :
            instance.parts
        ) {
            if (part.variantIndex >= variantHandles.size()) {
                continue;
            }

            const eld::graphics::RenderModel& model =
                resources.getModel(variantHandles[part.variantIndex]);

            CachedLocPart cachedPart;
            cachedPart.cameraDependent =
                part.drawMode == SceneLocDrawMode::WallDecorationDiagonalBoth;
            cachedPart.meshes.reserve(model.meshes.size());

            const SceneLocDrawTransform primaryDraw =
                sceneLocFixedDrawTransform(instance, part, true);
            const SceneLocDrawTransform alternateDraw =
                sceneLocFixedDrawTransform(instance, part, false);

            for (const eld::graphics::RenderMesh& mesh : model.meshes) {
                CachedLocMesh cachedMesh;
                cachedMesh.model = &model;
                cachedMesh.mesh = &mesh;
                cachedMesh.worldVertices.reserve(mesh.vertices.size());
                if (cachedPart.cameraDependent) {
                    cachedMesh.alternateWorldVertices.reserve(
                        mesh.vertices.size()
                    );
                }

                for (
                    const eld::graphics::RenderVertex& vertex :
                    mesh.vertices
                ) {
                    const Vec3 primary =
                        sceneLocModelPoint(instance, vertex, primaryDraw);
                    cachedMesh.worldVertices.push_back(primary);
                    includeBounds(primary);
                    ++cache.worldVertices;

                    if (cachedPart.cameraDependent) {
                        const Vec3 alternate =
                            sceneLocModelPoint(
                                instance,
                                vertex,
                                alternateDraw
                            );
                        cachedMesh.alternateWorldVertices.push_back(
                            alternate
                        );
                        includeBounds(alternate);
                        ++cache.alternateWorldVertices;
                    }
                }

                cachedPart.meshes.push_back(std::move(cachedMesh));
                ++cache.meshes;
            }

            cachedInstance.parts.push_back(std::move(cachedPart));
            ++cache.parts;
        }

        if (haveBounds) {
            cachedInstance.boundsCenter = Vec3{
                (boundsMin.x + boundsMax.x) * 0.5f,
                (boundsMin.y + boundsMax.y) * 0.5f,
                (boundsMin.z + boundsMax.z) * 0.5f
            };

            float radiusSquared = 0.0f;
            for (const CachedLocPart& part : cachedInstance.parts) {
                for (const CachedLocMesh& mesh : part.meshes) {
                    const auto includeRadius = [&](Vec3 point) {
                        const Vec3 delta = subtractVec3(
                            point,
                            cachedInstance.boundsCenter
                        );
                        radiusSquared = std::max(
                            radiusSquared,
                            dotVec3(delta, delta)
                        );
                    };

                    for (Vec3 point : mesh.worldVertices) {
                        includeRadius(point);
                    }
                    for (Vec3 point : mesh.alternateWorldVertices) {
                        includeRadius(point);
                    }
                }
            }
            cachedInstance.boundsRadius =
                std::max(0.05f, std::sqrt(radiusSquared));
        }
        else {
            cachedInstance.boundsCenter =
                sceneLocPoint(
                    instance.sceneX,
                    instance.sceneY,
                    instance.sceneZ
                );
            cachedInstance.boundsRadius = 0.25f;
        }

        cache.instances.push_back(std::move(cachedInstance));
    }

    return cache;
}

struct Map3dLocRenderStats {
    std::size_t instances = 0;
    std::size_t instancesCulled = 0;
    std::size_t instancesRendered = 0;
    std::size_t parts = 0;
    std::size_t meshes = 0;
    std::size_t verticesProjected = 0;
    std::size_t triangleCandidates = 0;
    std::size_t triangles = 0;
    std::size_t texturedTriangles = 0;
};

void appendSceneLocModelTriangles(
    std::vector<ProjectedTriangle>& triangles,
    const CachedLocScene& locCache,
    Map3dGraphicsTextureCache& textureCache,
    std::size_t scenePlane,
    const ProjectionContext& projection,
    std::vector<ProjectedPoint>& projectedScratch,
    Map3dLocRenderStats* renderStats
) {
    for (const CachedLocInstance& instance : locCache.instances) {
        if (instance.scenePlane != scenePlane) {
            continue;
        }

        if (renderStats != nullptr) {
            ++renderStats->instances;
        }

        if (!sphereVisible(
                instance.boundsCenter,
                instance.boundsRadius,
                projection
            )) {
            if (renderStats != nullptr) {
                ++renderStats->instancesCulled;
            }
            continue;
        }

        if (renderStats != nullptr) {
            ++renderStats->instancesRendered;
        }

        const bool diagonalInset =
            sceneLocDiagonalUsesInset(
                instance.rotation,
                instance.sceneX,
                instance.sceneZ,
                projection.camera
            );

        for (const CachedLocPart& part : instance.parts) {
            if (renderStats != nullptr) {
                ++renderStats->parts;
            }

            for (const CachedLocMesh& cachedMesh : part.meshes) {
                if (
                    cachedMesh.mesh == nullptr ||
                    cachedMesh.model == nullptr
                ) {
                    continue;
                }

                const eld::graphics::RenderMesh& mesh = *cachedMesh.mesh;
                const eld::graphics::RenderModel& model = *cachedMesh.model;
                const std::vector<Vec3>& worldVertices =
                    part.cameraDependent && !diagonalInset
                        ? cachedMesh.alternateWorldVertices
                        : cachedMesh.worldVertices;

                if (worldVertices.size() != mesh.vertices.size()) {
                    continue;
                }

                if (renderStats != nullptr) {
                    ++renderStats->meshes;
                    renderStats->verticesProjected += worldVertices.size();
                }

                projectedScratch.clear();
                if (projectedScratch.capacity() < worldVertices.size()) {
                    projectedScratch.reserve(worldVertices.size());
                }

                for (Vec3 point : worldVertices) {
                    projectedScratch.push_back(
                        projectPoint(point, projection)
                    );
                }

                for (
                    const eld::graphics::RenderMeshSection& section :
                    mesh.sections
                ) {
                    if (section.materialIndex >= model.materials.size()) {
                        continue;
                    }

                    const eld::graphics::RenderMaterial& material =
                        model.materials[section.materialIndex];

                    SDL_Texture* texture = nullptr;
                    if (material.texture.has_value()) {
                        texture = textureCache.get(*material.texture);
                    }

                    const std::size_t first = section.firstIndex;
                    const std::size_t end = std::min(
                        mesh.indices.size(),
                        first +
                            static_cast<std::size_t>(section.indexCount)
                    );

                    for (
                        std::size_t index = first;
                        index + 2 < end;
                        index += 3
                    ) {
                        if (renderStats != nullptr) {
                            ++renderStats->triangleCandidates;
                        }

                        const std::uint32_t ia = mesh.indices[index];
                        const std::uint32_t ib = mesh.indices[index + 1];
                        const std::uint32_t ic = mesh.indices[index + 2];

                        if (
                            ia >= mesh.vertices.size() ||
                            ib >= mesh.vertices.size() ||
                            ic >= mesh.vertices.size()
                        ) {
                            continue;
                        }

                        const ProjectedPoint& a = projectedScratch[ia];
                        const ProjectedPoint& b = projectedScratch[ib];
                        const ProjectedPoint& c = projectedScratch[ic];

                        if (!a.visible || !b.visible || !c.visible) {
                            continue;
                        }

                        const eld::graphics::RenderVertex& va =
                            mesh.vertices[ia];
                        const eld::graphics::RenderVertex& vb =
                            mesh.vertices[ib];
                        const eld::graphics::RenderVertex& vc =
                            mesh.vertices[ic];

                        ProjectedTriangle output;
                        output.vertices = {
                            SDL_Vertex{
                                SDL_FPoint{a.x, a.y},
                                renderVertexColor(va, material),
                                renderVertexUv(va, material)
                            },
                            SDL_Vertex{
                                SDL_FPoint{b.x, b.y},
                                renderVertexColor(vb, material),
                                renderVertexUv(vb, material)
                            },
                            SDL_Vertex{
                                SDL_FPoint{c.x, c.y},
                                renderVertexColor(vc, material),
                                renderVertexUv(vc, material)
                            }
                        };
                        output.texture = texture;
                        output.depth =
                            (a.depth + b.depth + c.depth) / 3.0f +
                            section.depthBias;
                        triangles.push_back(output);
                        if (renderStats != nullptr) {
                            ++renderStats->triangles;
                            if (texture != nullptr) {
                                ++renderStats->texturedTriangles;
                            }
                        }
                    }
                }
            }
        }
    }
}

ViewerColor sceneLocDebugColor(
    eld::graphics::map::SceneLocKind kind
) {
    using eld::graphics::map::SceneLocKind;

    switch (kind) {
        case SceneLocKind::Wall:
            return ViewerColor{255, 170, 70};
        case SceneLocKind::WallDecoration:
            return ViewerColor{255, 90, 220};
        case SceneLocKind::GroundDecoration:
            return ViewerColor{90, 255, 120};
        case SceneLocKind::Roof:
            return ViewerColor{255, 230, 80};
        case SceneLocKind::Location:
            return ViewerColor{80, 220, 255};
    }

    return ViewerColor{255, 255, 255};
}

void drawSceneLocLine(
    SDL_Renderer* renderer,
    Vec3 a,
    Vec3 b,
    ViewerColor color,
    Vec3 camera,
    Vec3 right,
    Vec3 up,
    Vec3 forward,
    int windowWidth,
    int windowHeight
) {
    const ProjectedPoint projectedA =
        projectPoint(
            a,
            camera,
            right,
            up,
            forward,
            windowWidth,
            windowHeight
        );
    const ProjectedPoint projectedB =
        projectPoint(
            b,
            camera,
            right,
            up,
            forward,
            windowWidth,
            windowHeight
        );

    if (!projectedA.visible || !projectedB.visible) {
        return;
    }

    SDL_SetRenderDrawColor(
        renderer,
        color.r,
        color.g,
        color.b,
        255
    );
    SDL_RenderLine(
        renderer,
        projectedA.x,
        projectedA.y,
        projectedB.x,
        projectedB.y
    );
}

void drawSceneLocDebug(
    SDL_Renderer* renderer,
    const eld::graphics::map::SceneLocPlacement& loc,
    Vec3 camera,
    Vec3 right,
    Vec3 up,
    Vec3 forward,
    int windowWidth,
    int windowHeight
) {
    constexpr float RegionCenter =
        static_cast<float>(RegionSize) * 0.5f;
    constexpr float Pi = 3.14159265358979323846f;

    const ViewerColor color =
        sceneLocDebugColor(loc.kind);
    const float baseY = terrainWorldHeight(loc.sceneY) + 0.025f;

    const float minX =
        static_cast<float>(loc.tileX) - RegionCenter;
    const float minZ =
        static_cast<float>(loc.tileZ) - RegionCenter;
    const float maxX =
        minX + static_cast<float>(loc.footprintWidth);
    const float maxZ =
        minZ + static_cast<float>(loc.footprintLength);

    const Vec3 p00{minX, baseY, minZ};
    const Vec3 p10{maxX, baseY, minZ};
    const Vec3 p11{maxX, baseY, maxZ};
    const Vec3 p01{minX, baseY, maxZ};

    const auto line = [&](Vec3 a, Vec3 b) {
        drawSceneLocLine(
            renderer,
            a,
            b,
            color,
            camera,
            right,
            up,
            forward,
            windowWidth,
            windowHeight
        );
    };

    const Vec3 anchor = sceneLocPoint(
        loc.sceneX,
        loc.sceneY,
        loc.sceneZ
    );

    using eld::graphics::map::SceneLocKind;

    if (loc.kind == SceneLocKind::GroundDecoration) {
        constexpr float Half = 0.16f;
        line(
            Vec3{anchor.x - Half, anchor.y + 0.03f, anchor.z},
            Vec3{anchor.x + Half, anchor.y + 0.03f, anchor.z}
        );
        line(
            Vec3{anchor.x, anchor.y + 0.03f, anchor.z - Half},
            Vec3{anchor.x, anchor.y + 0.03f, anchor.z + Half}
        );
        line(
            anchor,
            Vec3{anchor.x, anchor.y + 0.22f, anchor.z}
        );
        return;
    }

    if (loc.kind == SceneLocKind::WallDecoration) {
        constexpr float Half = 0.12f;
        line(
            Vec3{anchor.x - Half, anchor.y + 0.04f, anchor.z - Half},
            Vec3{anchor.x + Half, anchor.y + 0.04f, anchor.z + Half}
        );
        line(
            Vec3{anchor.x - Half, anchor.y + 0.04f, anchor.z + Half},
            Vec3{anchor.x + Half, anchor.y + 0.04f, anchor.z - Half}
        );
        line(
            anchor,
            Vec3{anchor.x, anchor.y + 0.42f, anchor.z}
        );
    }
    else {
        line(p00, p10);
        line(p10, p11);
        line(p11, p01);
        line(p01, p00);

        const float markerHeight =
            loc.kind == SceneLocKind::Roof ? 0.28f : 0.55f;

        if (loc.kind == SceneLocKind::Location) {
            const Vec3 q00{p00.x, p00.y + markerHeight, p00.z};
            const Vec3 q10{p10.x, p10.y + markerHeight, p10.z};
            const Vec3 q11{p11.x, p11.y + markerHeight, p11.z};
            const Vec3 q01{p01.x, p01.y + markerHeight, p01.z};

            line(p00, q00);
            line(p10, q10);
            line(p11, q11);
            line(p01, q01);
            line(q00, q10);
            line(q10, q11);
            line(q11, q01);
            line(q01, q00);
        }
        else {
            line(
                anchor,
                Vec3{anchor.x, anchor.y + markerHeight, anchor.z}
            );
        }
    }

    // Type 9 is a one-tile diagonal loc. Show the diagonal explicitly because
    // a plain 1x1 footprint would hide its most important placement property.
    if (loc.shape == 9) {
        if ((loc.rotation & 1u) == 0) {
            line(p00, p11);
        }
        else {
            line(p10, p01);
        }
    }

    // Build a diagnostic heading from the model rotation plus any extra
    // World3D yaw. Wall decorations are special: their model is requested at
    // rotation 0 and World3D carries the orientation separately.
    int debugYaw =
        static_cast<int>(loc.primaryModelRotation & 3u) *
        eld::graphics::map::SceneLocBuilder::QuarterTurn +
        loc.sceneYaw;

    if (loc.kind == SceneLocKind::WallDecoration) {
        if (loc.shape == 4 || loc.shape == 5) {
            debugYaw = loc.decorationAngle;
        }
        else {
            // Shapes 6..8 use camera-facing 0x100/0x200 decoration modes; a
            // quarter-turn ray is the stable placement orientation to show.
            debugYaw =
                static_cast<int>(loc.rotation) *
                eld::graphics::map::SceneLocBuilder::QuarterTurn;
        }
    }

    const float angle =
        static_cast<float>(debugYaw) *
        (2.0f * Pi / 2048.0f);
    constexpr float ArrowLength = 0.42f;
    const Vec3 arrowStart{
        anchor.x,
        anchor.y + 0.08f,
        anchor.z
    };
    const Vec3 arrowEnd{
        arrowStart.x + std::sin(angle) * ArrowLength,
        arrowStart.y,
        arrowStart.z + std::cos(angle) * ArrowLength
    };
    line(arrowStart, arrowEnd);

    // L walls have two separately rotated model faces.
    if (loc.shape == 2 && loc.hasSecondaryModel) {
        const float secondAngle =
            static_cast<float>(loc.secondaryModelRotation & 3u) *
            (Pi * 0.5f);
        line(
            arrowStart,
            Vec3{
                arrowStart.x + std::sin(secondAngle) * ArrowLength,
                arrowStart.y,
                arrowStart.z + std::cos(secondAngle) * ArrowLength
            }
        );
    }
}

struct TerrainNeighborhoodRegion {
    int offsetX = 0;
    int offsetY = 0;
    eld::map::MapRegion region;
};

struct TerrainNeighborhood {
    std::vector<TerrainNeighborhoodRegion> regions;
    std::vector<std::uint16_t> missingRegionIds;
};

TerrainNeighborhood loadTerrainNeighborhood(
    const eld::map::MapLoader& loader,
    const eld::map::MapIndexEntry& center
) {
    TerrainNeighborhood neighborhood;
    neighborhood.regions.reserve(9);

    for (int offsetX = -1; offsetX <= 1; ++offsetX) {
        for (int offsetY = -1; offsetY <= 1; ++offsetY) {
            const int regionX = center.regionX() + offsetX;
            const int regionY = center.regionY() + offsetY;

            if (
                regionX < 0 || regionX > 255 ||
                regionY < 0 || regionY > 255
            ) {
                continue;
            }

            const std::uint16_t candidateId =
                static_cast<std::uint16_t>(
                    (regionX << 8) | regionY
                );

            if (loader.find(candidateId) == nullptr) {
                neighborhood.missingRegionIds.push_back(candidateId);
                continue;
            }

            neighborhood.regions.push_back({
                offsetX,
                offsetY,
                loader.loadTerrain(candidateId)
            });
        }
    }

    return neighborhood;
}

const eld::map::MapRegion* neighborhoodTerrain(
    const TerrainNeighborhood& neighborhood,
    int offsetX,
    int offsetY
) {
    for (
        const TerrainNeighborhoodRegion& region :
        neighborhood.regions
    ) {
        if (
            region.offsetX == offsetX &&
            region.offsetY == offsetY
        ) {
            return &region.region;
        }
    }

    return nullptr;
}

int regionOffsetForLocalCoordinate(
    int coordinate
) {
    if (coordinate >= 0) {
        return coordinate / static_cast<int>(RegionSize);
    }

    return
        -(
            (-coordinate + static_cast<int>(RegionSize) - 1) /
            static_cast<int>(RegionSize)
        );
}

const eld::map::MapTile* sampleTerrainNeighborhood(
    const TerrainNeighborhood& neighborhood,
    std::size_t plane,
    int x,
    int y
) {
    if (plane >= eld::map::PlaneCount) {
        return nullptr;
    }

    const int offsetX = regionOffsetForLocalCoordinate(x);
    const int offsetY = regionOffsetForLocalCoordinate(y);

    if (
        offsetX < -1 || offsetX > 1 ||
        offsetY < -1 || offsetY > 1
    ) {
        return nullptr;
    }

    const eld::map::MapRegion* terrain =
        neighborhoodTerrain(
            neighborhood,
            offsetX,
            offsetY
        );

    if (terrain == nullptr) {
        return nullptr;
    }

    const int localX =
        x - offsetX * static_cast<int>(RegionSize);
    const int localY =
        y - offsetY * static_cast<int>(RegionSize);

    if (
        localX < 0 || localX >= static_cast<int>(RegionSize) ||
        localY < 0 || localY >= static_cast<int>(RegionSize)
    ) {
        return nullptr;
    }

    return &terrain->tile(
        plane,
        static_cast<std::size_t>(localX),
        static_cast<std::size_t>(localY)
    );
}


std::size_t countTileFlag(
    const eld::map::MapRegion& region,
    eld::map::TileFlag flag
) {
    std::size_t count = 0;

    for (std::size_t plane = 0; plane < eld::map::PlaneCount; ++plane) {
        for (std::size_t x = 0; x < eld::map::RegionSize; ++x) {
            for (std::size_t y = 0; y < eld::map::RegionSize; ++y) {
                if (region.tile(plane, x, y).hasFlag(flag)) {
                    ++count;
                }
            }
        }
    }

    return count;
}

std::optional<std::size_t> sceneSourcePlane(
    const TerrainNeighborhood& neighborhood,
    std::size_t scenePlane,
    int x,
    int y
) {
    const eld::map::MapTile* levelOne =
        sampleTerrainNeighborhood(
            neighborhood,
            1,
            x,
            y
        );

    const std::uint8_t levelOneSettings =
        levelOne == nullptr ? 0 : levelOne->settings;

    return eld::graphics::map::ClassicTileRules::
        sourcePlaneForScenePlane(
            scenePlane,
            levelOneSettings
        );
}

bool sceneTileHasBridgeGround(
    const TerrainNeighborhood& neighborhood,
    std::size_t scenePlane,
    int x,
    int y
) {
    const eld::map::MapTile* levelOne =
        sampleTerrainNeighborhood(
            neighborhood,
            1,
            x,
            y
        );

    return eld::graphics::map::ClassicTileRules::
        hasBridgeGround(
            scenePlane,
            levelOne == nullptr ? 0 : levelOne->settings
        );
}

struct Map3dPlaneStats {
    int minHeight = std::numeric_limits<int>::max();
    int maxHeight = std::numeric_limits<int>::min();
    std::int64_t heightSum = 0;
    std::size_t tiles = 0;
};

Map3dPlaneStats map3dPlaneStats(
    const eld::map::MapRegion& region,
    std::size_t plane
) {
    Map3dPlaneStats stats;

    for (std::size_t x = 0; x < RegionSize; ++x) {
        for (std::size_t y = 0; y < RegionSize; ++y) {
            const int height = region.tile(plane, x, y).height;
            stats.minHeight = std::min(stats.minHeight, height);
            stats.maxHeight = std::max(stats.maxHeight, height);
            stats.heightSum += height;
            ++stats.tiles;
        }
    }

    return stats;
}

std::size_t mostInteresting3dPlane(
    const eld::map::MapRegion& terrain
) {
    std::size_t bestPlane = 0;
    int bestRange = -1;

    for (std::size_t plane = 0; plane < TerrainPlanes; ++plane) {
        const Map3dPlaneStats stats =
            map3dPlaneStats(terrain, plane);
        const int range = stats.maxHeight - stats.minHeight;

        if (range > bestRange) {
            bestRange = range;
            bestPlane = plane;
        }
    }

    return bestPlane;
}

std::string viewer3dTitle(
    const MapIndexEntry& region,
    const Map3dViewerState& state
) {
    std::ostringstream stream;
    stream
        << "Eldoria Map 3D Probe | region "
        << region.regionId
        << " ("
        << region.regionX()
        << ","
        << region.regionY()
        << ") | scene plane "
        << state.plane
        << " | arrows orbit/pitch | +/- zoom | G grid "
        << (state.showGrid ? "on" : "off")
        << " | M models "
        << (state.showLocModels ? "on" : "off")
        << " | L debug "
        << (state.showLocs ? "on" : "off");
    return stream.str();
}

void viewRegion3d(
    const eld::cache::Cache& cache,
    const eld::cache::Store& maps,
    const std::vector<MapIndexEntry>& mapIndex,
    std::uint16_t regionId
) {
    const MapIndexEntry* region =
        findRegion(mapIndex, regionId);

    if (region == nullptr) {
        throw std::runtime_error(
            "map_index has no region " +
            std::to_string(regionId)
        );
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
    eld::texture::TextureRepository textures(
        cache.open(eld::cache::IndexId::Config)
    );
    eld::model::ModelRepository models(
        cache.open(eld::cache::IndexId::Models)
    );

    // view3d now consumes the production data/map path. The older probe
    // decoders remain below for deep/validate diagnostics only.
    (void)maps;
    (void)mapIndex;
    const eld::map::MapLoader dataMapLoader(cache);
    const eld::map::MapIndexEntry* dataRegion =
        dataMapLoader.find(regionId);
    if (dataRegion == nullptr) {
        throw std::runtime_error(
            "production data/map loader has no region " +
            std::to_string(regionId)
        );
    }

    const TerrainNeighborhood neighborhood =
        loadTerrainNeighborhood(
            dataMapLoader,
            *dataRegion
        );
    const eld::map::MapRegion* centerTerrain =
        neighborhoodTerrain(
            neighborhood,
            0,
            0
        );

    if (centerTerrain == nullptr) {
        throw std::runtime_error(
            "center terrain disappeared while loading neighborhood"
        );
    }

    const eld::map::MapRegion centerRegion =
        dataMapLoader.load(regionId);

    const eld::graphics::map::SceneLocTileSampler locSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    eld::graphics::map::SceneLocBuilder locBuilder;
    const std::vector<eld::graphics::map::SceneLocPlacement> sceneLocs =
        locBuilder.build(
            centerRegion.objects,
            locations,
            locSampler
        );

    eld::graphics::map::SceneLocModelBuilder locModelBuilder;
    const eld::graphics::map::SceneLocModelBuildResult locModels =
        locModelBuilder.build(
            sceneLocs,
            locations,
            models
        );

    eld::graphics::GraphicsResources graphicsResources(
        models,
        textures
    );
    std::vector<eld::graphics::ModelHandle> locVariantHandles;
    locVariantHandles.reserve(locModels.variants.size());

    for (
        const eld::graphics::map::SceneLocModelVariant& variant :
        locModels.variants
    ) {
        locVariantHandles.push_back(
            graphicsResources.resolveModel(variant.mesh)
        );
    }

    std::array<std::size_t, eld::map::PlaneCount> sceneLocPlaneCounts{};
    std::array<std::size_t, 5> sceneLocKindCounts{};
    std::size_t bridgeAttachmentLocs = 0;

    for (const auto& loc : sceneLocs) {
        if (loc.scenePlane < sceneLocPlaneCounts.size()) {
            ++sceneLocPlaneCounts[loc.scenePlane];
        }
        ++sceneLocKindCounts[static_cast<std::size_t>(loc.kind)];
        if (loc.bridgeAttachment) {
            ++bridgeAttachmentLocs;
        }
    }

    eld::graphics::map::SceneTerrainBuilder terrainBuilder;
    eld::graphics::map::ClassicTerrainAppearanceBuilder
        appearanceBuilder;

    Map3dViewerState state;
    state.plane = mostInteresting3dPlane(*centerTerrain);

    std::cout
        << "MAP 3D SCENE-TILE PROBE\n"
        << "=======================\n"
        << "region: " << region->regionId
        << " (" << region->regionX()
        << "," << region->regionY() << ")\n"
        << "world base: x=" << region->regionX() * 64
        << " y=" << region->regionY() * 64 << '\n'
        << "terrain file: " << region->terrainFileId << '\n'
        << "neighborhood regions loaded: "
        << neighborhood.regions.size()
        << "/9\n"
        << "starting scene plane: " << state.plane << "\n"
        << "tile flags (all decoded planes):"
        << " solid="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::Solid)
        << " bridge="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::Bridge)
        << " roof="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::Roof)
        << " force0="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::ForceLevelZero)
        << " lowmem="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::LowMemoryHidden)
        << " unknown20="
        << countTileFlag(*centerTerrain, eld::map::TileFlag::Unknown20)
        << '\n'
        << "loc placements: decoded=" << centerRegion.objects.size()
        << " built=" << sceneLocs.size()
        << " scene-planes="
        << sceneLocPlaneCounts[0] << ','
        << sceneLocPlaneCounts[1] << ','
        << sceneLocPlaneCounts[2] << ','
        << sceneLocPlaneCounts[3]
        << " bridge-ground=" << bridgeAttachmentLocs
        << '\n'
        << "loc kinds: walls=" << sceneLocKindCounts[0]
        << " wall-decor=" << sceneLocKindCounts[1]
        << " ground-decor=" << sceneLocKindCounts[2]
        << " locs=" << sceneLocKindCounts[3]
        << " roofs=" << sceneLocKindCounts[4]
        << '\n'
        << "loc models: instances=" << locModels.stats.instances
        << '/' << locModels.stats.placements
        << " parts=" << locModels.stats.parts
        << " variants=" << locModels.stats.variants
        << " missing-shape=" << locModels.stats.missingShapeModels
        << " missing-model-file=" << locModels.stats.missingModelFiles
        << " contoured=" << locModels.stats.contouredGround
        << " animated-static=" << locModels.stats.animated
        << "\n\n"
        << "Controls\n"
        << "--------\n"
        << "1 / 2 / 3 / 4 : scene plane 0 / 1 / 2 / 3\n"
        << "Left / Right   : orbit\n"
        << "Up / Down      : camera pitch\n"
        << "+ / -          : zoom in / out\n"
        << "Mouse wheel    : zoom\n"
        << "U              : toggle underlay triangles\n"
        << "O              : toggle overlay triangles\n"
        << "G              : toggle shaped-triangle edges\n"
        << "M              : toggle real loc models\n"
        << "L              : toggle loc debug markers\n"
        << "F3             : toggle perf debug panel\n"
        << "F9             : dump perf debug report txt\n"
        << "R              : reset camera\n"
        << "Esc            : quit\n\n"
        << "This probe now uses production data/map decoding plus the\n"
        << "classic 13 scene-tile shapes and classic terrain HSL lighting.\n"
        << "Cache overlay shape 0..11 maps to scene shape 1..12;\n"
        << "scene shape 0 is a plain tile with no overlay.\n"
        << "FF00FF invisible overlays are omitted. Textured floors load\n"
        << "their actual decoded cache images through data/texture.\n"
        << "Bridge tiles now use classic World3D::setBridge plane shifting;\n"
        << "scene plane 0 also keeps/draws the original plane-0 ground below.\n"
        << "Loc models use SceneLoc placement -> data/model -> GraphicsResources;\n"
        << "M toggles real cached models and L keeps the placement debug overlay.\n";

    if (!neighborhood.missingRegionIds.empty()) {
        std::cout
            << "Missing neighborhood region ids:";
        for (
            std::uint16_t missing :
            neighborhood.missingRegionIds
        ) {
            std::cout << ' ' << missing;
        }
        std::cout
            << "\nBorder tiles needing those height samples are omitted.\n";
    }

    eld::platform::SdlContext sdl(
        "Eldoria Map 3D Scene-Tile Probe",
        1100,
        800
    );

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (window == nullptr || renderer == nullptr) {
        throw std::runtime_error(
            "failed to create SDL 3D map viewer"
        );
    }

    Map3dTextureCache textureCache(renderer, textures);
    Map3dGraphicsTextureCache locTextureCache(
        renderer,
        graphicsResources
    );

    // Build all source-dependent terrain geometry once. Camera projection is
    // still per-frame, but floor blending, shaped-tile construction, colors,
    // UVs and texture lookup no longer repeat at 4-15 FPS.
    std::array<CachedTerrainPlane, eld::map::PlaneCount> terrainCache;
    const eld::graphics::map::TerrainTileSampler terrainSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    const auto cacheTerrainTile = [&] (
        CachedTerrainPlane& cachedPlane,
        std::size_t sourcePlane,
        std::size_t x,
        std::size_t y
    ) {
        const eld::map::MapTile* sw = sampleTerrainNeighborhood(
            neighborhood,
            sourcePlane,
            static_cast<int>(x),
            static_cast<int>(y)
        );
        const eld::map::MapTile* se = sampleTerrainNeighborhood(
            neighborhood,
            sourcePlane,
            static_cast<int>(x) + 1,
            static_cast<int>(y)
        );
        const eld::map::MapTile* ne = sampleTerrainNeighborhood(
            neighborhood,
            sourcePlane,
            static_cast<int>(x) + 1,
            static_cast<int>(y) + 1
        );
        const eld::map::MapTile* nw = sampleTerrainNeighborhood(
            neighborhood,
            sourcePlane,
            static_cast<int>(x),
            static_cast<int>(y) + 1
        );

        if (sw == nullptr || se == nullptr || ne == nullptr || nw == nullptr) {
            return;
        }

        const eld::graphics::map::SceneTileAppearance appearance =
            appearanceBuilder.build(
                sourcePlane,
                static_cast<int>(x),
                static_cast<int>(y),
                terrainSampler,
                floors
            );

        if (!appearance.underlayVisible && !appearance.overlayVisible) {
            return;
        }

        std::uint8_t sceneShape = 0;
        std::uint8_t rotation = 0;
        if (sw->overlayId != 0) {
            if (sw->overlayShape >= 12) {
                return;
            }
            sceneShape = static_cast<std::uint8_t>(sw->overlayShape + 1u);
            rotation = static_cast<std::uint8_t>(sw->overlayRotation & 3u);
        }

        const eld::graphics::map::SceneTileMesh mesh =
            terrainBuilder.buildTile(
                static_cast<int>(x),
                static_cast<int>(y),
                sceneShape,
                rotation,
                eld::graphics::map::TerrainCornerHeights{
                    sw->height,
                    se->height,
                    ne->height,
                    nw->height
                },
                appearance.shades
            );

        CachedTerrainTile cachedTile;
        cachedTile.vertices.reserve(mesh.vertices.size());
        cachedTile.triangles.reserve(mesh.triangles.size());

        for (const auto& vertex : mesh.vertices) {
            cachedTile.vertices.push_back(sceneTerrainPoint(vertex));
        }

        for (const auto& triangle : mesh.triangles) {
            if (
                triangle.surface == eld::graphics::map::TerrainSurface::Underlay &&
                !appearance.underlayVisible
            ) {
                continue;
            }
            if (
                triangle.surface == eld::graphics::map::TerrainSurface::Overlay &&
                !appearance.overlayVisible
            ) {
                continue;
            }

            const bool texturedOverlay =
                triangle.surface == eld::graphics::map::TerrainSurface::Overlay &&
                appearance.textureId.has_value();

            CachedTerrainTriangle cachedTriangle;
            cachedTriangle.indices = triangle.indices;
            cachedTriangle.surface = triangle.surface;
            if (texturedOverlay) {
                cachedTriangle.texture = textureCache.get(
                    static_cast<std::uint16_t>(*appearance.textureId)
                );
            }

            for (std::size_t corner = 0; corner < 3; ++corner) {
                const std::uint8_t vertexIndex = triangle.indices[corner];
                const auto& vertex = mesh.vertices[vertexIndex];
                const int shade =
                    triangle.surface == eld::graphics::map::TerrainSurface::Overlay
                        ? vertex.overlayShade
                        : vertex.underlayShade;
                const std::uint32_t rgb = texturedOverlay
                    ? eld::graphics::map::ClassicTerrainAppearanceBuilder::
                          textureModulationRgb(shade)
                    : eld::graphics::map::ClassicTerrainAppearanceBuilder::
                          paletteRgb(shade);

                cachedTriangle.colors[corner] = toSdlColorRgb(rgb);
                cachedTriangle.uvs[corner] = texturedOverlay
                    ? SDL_FPoint{
                          triangle.textureUvs[corner].u,
                          triangle.textureUvs[corner].v
                      }
                    : SDL_FPoint{0.0f, 0.0f};
            }

            cachedTile.triangles.push_back(cachedTriangle);
            ++cachedPlane.triangles;
        }

        cachedPlane.sourceVertices += cachedTile.vertices.size();
        cachedPlane.tiles.push_back(std::move(cachedTile));
    };

    for (std::size_t scenePlane = 0; scenePlane < eld::map::PlaneCount; ++scenePlane) {
        CachedTerrainPlane& cachedPlane = terrainCache[scenePlane];
        cachedPlane.tiles.reserve(RegionSize * RegionSize + 64);

        for (std::size_t x = 0; x < RegionSize; ++x) {
            for (std::size_t y = 0; y < RegionSize; ++y) {
                const int tileX = static_cast<int>(x);
                const int tileY = static_cast<int>(y);

                if (
                    sceneTileHasBridgeGround(
                        neighborhood,
                        scenePlane,
                        tileX,
                        tileY
                    )
                ) {
                    cacheTerrainTile(cachedPlane, 0, x, y);
                }

                const std::optional<std::size_t> sourcePlane =
                    sceneSourcePlane(
                        neighborhood,
                        scenePlane,
                        tileX,
                        tileY
                    );
                if (sourcePlane.has_value()) {
                    cacheTerrainTile(cachedPlane, *sourcePlane, x, y);
                }
            }
        }
    }

    const CachedLocScene locSceneCache = buildCachedLocScene(
        locModels,
        locVariantHandles,
        graphicsResources
    );

    std::cout
        << "static render cache: terrain tiles="
        << terrainCache[state.plane].tiles.size()
        << " terrain tris=" << terrainCache[state.plane].triangles
        << " loc world verts=" << locSceneCache.worldVertices
        << " alt wall-decor verts=" << locSceneCache.alternateWorldVertices
        << '\n';

    // Reuse the large transient render buffers instead of allocating and
    // freeing hundreds of thousands of elements every frame.
    std::vector<ProjectedTriangle> triangles;
    triangles.reserve(250000);
    std::vector<SDL_Vertex> geometry;
    geometry.reserve(700000);
    std::vector<ProjectedPoint> projectedScratch;
    projectedScratch.reserve(4096);

    bool running = true;
    bool titleDirty = true;

    using FpsClock = std::chrono::steady_clock;
    auto fpsSampleStart = FpsClock::now();
    std::size_t fpsSampleFrames = 0;
    double fps = 0.0;
    double frameTimeMs = 0.0;

    Map3dFrameTimings timingSum;
    Map3dFrameTimings timingAverage;
    Map3dFrameTimings lastTiming;
    Map3dFrameCounters lastCounters;
    std::size_t timingFrames = 0;

    bool dumpRequested = false;
    std::string dumpStatus;
    const std::filesystem::path debugReportPath =
        "map_probe_debug_region_" +
        std::to_string(regionId) +
        ".txt";

    const auto writeDebugReport = [&] (
        int windowWidth,
        int windowHeight
    ) {
        std::ofstream output(debugReportPath);
        if (!output.is_open()) {
            dumpStatus = "DUMP FAILED - CHECK TERMINAL";
            std::cerr
                << "Failed to open debug report: "
                << debugReportPath.string()
                << '\n';
            return;
        }

        const char* rendererName =
            SDL_GetRendererName(renderer);

        output
            << "ELDORIA MAP_PROBE PERFORMANCE DEBUG REPORT\n"
            << "===========================================\n\n"
            << "region_id: " << regionId << '\n'
            << "region_xy: " << region->regionX() << ',' << region->regionY() << '\n'
            << "world_base: " << region->regionX() * 64 << ',' << region->regionY() * 64 << '\n'
            << "terrain_file: " << region->terrainFileId << '\n'
            << "object_file: " << region->objectFileId << '\n'
            << "neighborhood_loaded: " << neighborhood.regions.size() << "/9\n"
            << "renderer: " << (rendererName == nullptr ? "unknown" : rendererName) << '\n'
            << "window: " << windowWidth << 'x' << windowHeight << "\n\n"

            << "VIEW STATE\n"
            << "----------\n"
            << "scene_plane: " << state.plane << '\n'
            << "yaw: " << state.yaw << '\n'
            << "pitch: " << state.pitch << '\n'
            << "distance: " << state.distance << '\n'
            << "underlays: " << (state.showUnderlays ? "on" : "off") << '\n'
            << "overlays: " << (state.showOverlays ? "on" : "off") << '\n'
            << "grid: " << (state.showGrid ? "on" : "off") << '\n'
            << "loc_models: " << (state.showLocModels ? "on" : "off") << '\n'
            << "loc_debug: " << (state.showLocs ? "on" : "off") << "\n\n"

            << std::fixed << std::setprecision(3)
            << "PERFORMANCE SAMPLE\n"
            << "------------------\n"
            << "fps: " << fps << '\n'
            << "sampled_frame_ms: " << frameTimeMs << '\n'
            << "average_cpu_frame_ms: " << timingAverage.cpuFrameMs << '\n'
            << "average_input_ms: " << timingAverage.inputMs << '\n'
            << "average_camera_ms: " << timingAverage.cameraMs << '\n'
            << "average_terrain_ms: " << timingAverage.terrainMs << '\n'
            << "average_loc_models_ms: " << timingAverage.locModelsMs << '\n'
            << "average_sort_ms: " << timingAverage.sortMs << '\n'
            << "average_submit_ms: " << timingAverage.submitMs << '\n'
            << "average_debug_draw_ms: " << timingAverage.debugDrawMs << '\n'
            << "average_panel_ms: " << timingAverage.panelMs << '\n'
            << "average_present_ms: " << timingAverage.presentMs << "\n\n"

            << "LAST FRAME TIMINGS\n"
            << "------------------\n"
            << "cpu_frame_ms: " << lastTiming.cpuFrameMs << '\n'
            << "input_ms: " << lastTiming.inputMs << '\n'
            << "camera_ms: " << lastTiming.cameraMs << '\n'
            << "terrain_ms: " << lastTiming.terrainMs << '\n'
            << "loc_models_ms: " << lastTiming.locModelsMs << '\n'
            << "sort_ms: " << lastTiming.sortMs << '\n'
            << "submit_ms: " << lastTiming.submitMs << '\n'
            << "debug_draw_ms: " << lastTiming.debugDrawMs << '\n'
            << "panel_ms: " << lastTiming.panelMs << '\n'
            << "present_ms: " << lastTiming.presentMs << "\n\n"

            << "LAST FRAME COUNTERS\n"
            << "-------------------\n"
            << "terrain_tiles: " << lastCounters.terrainTiles << '\n'
            << "terrain_vertices_projected: " << lastCounters.terrainVerticesProjected << '\n'
            << "terrain_triangles: " << lastCounters.terrainTriangles << '\n'
            << "terrain_textured_triangles: " << lastCounters.terrainTexturedTriangles << '\n'
            << "loc_instances_plane_candidates: " << lastCounters.locInstances << '\n'
            << "loc_instances_culled: " << lastCounters.locInstancesCulled << '\n'
            << "loc_instances_rendered: " << lastCounters.locInstancesRendered << '\n'
            << "loc_parts: " << lastCounters.locParts << '\n'
            << "loc_meshes: " << lastCounters.locMeshes << '\n'
            << "loc_vertices_projected: " << lastCounters.locVerticesProjected << '\n'
            << "loc_triangle_candidates: " << lastCounters.locTriangleCandidates << '\n'
            << "loc_triangles_emitted: " << lastCounters.locTriangles << '\n'
            << "loc_textured_triangles: " << lastCounters.locTexturedTriangles << '\n'
            << "total_triangles: " << lastCounters.totalTriangles << '\n'
            << "geometry_vertices_submitted: " << lastCounters.geometryVertices << '\n'
            << "geometry_batches: " << lastCounters.geometryBatches << "\n\n"

            << "STATIC SCENE COUNTS\n"
            << "-------------------\n"
            << "decoded_loc_placements: " << centerRegion.objects.size() << '\n'
            << "scene_loc_placements: " << sceneLocs.size() << '\n'
            << "scene_loc_planes: "
            << sceneLocPlaneCounts[0] << ','
            << sceneLocPlaneCounts[1] << ','
            << sceneLocPlaneCounts[2] << ','
            << sceneLocPlaneCounts[3] << '\n'
            << "bridge_attachment_locs: " << bridgeAttachmentLocs << '\n'
            << "loc_kind_walls: " << sceneLocKindCounts[0] << '\n'
            << "loc_kind_wall_decor: " << sceneLocKindCounts[1] << '\n'
            << "loc_kind_ground_decor: " << sceneLocKindCounts[2] << '\n'
            << "loc_kind_locations: " << sceneLocKindCounts[3] << '\n'
            << "loc_kind_roofs: " << sceneLocKindCounts[4] << '\n'
            << "loc_model_instances: " << locModels.stats.instances << '\n'
            << "loc_model_placements: " << locModels.stats.placements << '\n'
            << "loc_model_parts: " << locModels.stats.parts << '\n'
            << "loc_model_variants: " << locModels.stats.variants << '\n'
            << "loc_model_missing_shape: " << locModels.stats.missingShapeModels << '\n'
            << "loc_model_missing_file: " << locModels.stats.missingModelFiles << '\n'
            << "loc_model_contoured: " << locModels.stats.contouredGround << '\n'
            << "loc_model_animated_static: " << locModels.stats.animated << '\n'
            << "terrain_textures_uploaded: " << textureCache.count() << '\n'
            << "loc_textures_uploaded: " << locTextureCache.count() << '\n'
            << "cached_terrain_tiles_current_plane: "
            << terrainCache[state.plane].tiles.size() << '\n'
            << "cached_terrain_source_vertices_current_plane: "
            << terrainCache[state.plane].sourceVertices << '\n'
            << "cached_terrain_triangles_current_plane: "
            << terrainCache[state.plane].triangles << '\n'
            << "cached_loc_instances: " << locSceneCache.instances.size() << '\n'
            << "cached_loc_parts: " << locSceneCache.parts << '\n'
            << "cached_loc_meshes: " << locSceneCache.meshes << '\n'
            << "cached_loc_world_vertices: " << locSceneCache.worldVertices << '\n'
            << "cached_loc_alternate_vertices: "
            << locSceneCache.alternateWorldVertices << "\n\n"

            << "TILE FLAGS (CENTER REGION, ALL DECODED PLANES)\n"
            << "---------------------------------------------\n"
            << "solid: " << countTileFlag(*centerTerrain, eld::map::TileFlag::Solid) << '\n'
            << "bridge: " << countTileFlag(*centerTerrain, eld::map::TileFlag::Bridge) << '\n'
            << "roof: " << countTileFlag(*centerTerrain, eld::map::TileFlag::Roof) << '\n'
            << "force_level_zero: " << countTileFlag(*centerTerrain, eld::map::TileFlag::ForceLevelZero) << '\n'
            << "low_memory_hidden: " << countTileFlag(*centerTerrain, eld::map::TileFlag::LowMemoryHidden) << '\n'
            << "unknown_0x20: " << countTileFlag(*centerTerrain, eld::map::TileFlag::Unknown20) << "\n\n"

            << "CURRENT PROBE RENDER PATH\n"
            << "-------------------------\n"
            << "1. Terrain scene-tile geometry/colors/UVs are cached once at viewer startup.\n"
            << "2. Terrain world vertices are CPU-projected each frame.\n"
            << "3. Loc definition/model transforms + contouring are cached as world-space vertices once.\n"
            << "4. Per-instance bounding spheres reject off-screen locs before vertex projection.\n"
            << "5. FOV/focal-length math is computed once per frame, not once per vertex.\n"
            << "6. Large triangle/geometry/projection scratch buffers are reused across frames.\n"
            << "7. Visible terrain + loc faces still expand into one ProjectedTriangle vector.\n"
            << "8. A global painter-sort still sorts all emitted triangles by average depth.\n"
            << "9. Consecutive same-texture runs still submit through SDL_RenderGeometry.\n"
            << "10. No occlusion culling or hardware depth buffer yet.\n\n"

            << "QUICK INTERPRETATION\n"
            << "--------------------\n"
            << "High terrain_ms now mostly means terrain projection/emission cost.\n"
            << "High loc_models_ms now means visible-instance projection/triangle emission cost.\n"
            << "Compare loc_instances_culled vs rendered to judge frustum-culling effectiveness.\n"
            << "High sort_ms: the global painter sort is the next architectural bottleneck.\n"
            << "High submit_ms or many geometry_batches: texture-fragmented SDL submission is expensive.\n"
            << "High present_ms: backend/vsync/display presentation is blocking.\n";

        output.flush();
        if (!output.good()) {
            dumpStatus = "DUMP FAILED - CHECK TERMINAL";
            std::cerr
                << "Failed while writing debug report: "
                << debugReportPath.string()
                << '\n';
            return;
        }

        dumpStatus = "WROTE " + debugReportPath.filename().string();
        std::cout
            << "Debug report written: "
            << std::filesystem::absolute(debugReportPath).string()
            << '\n';
    };

    while (running) {
        const auto frameStart = FpsClock::now();
        Map3dFrameTimings frameTiming;
        Map3dFrameCounters frameCounters;

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                state.distance = std::clamp(
                    state.distance - event.wheel.y * 4.0f,
                    24.0f,
                    180.0f
                );
                titleDirty = true;
                continue;
            }

            if (
                event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                event.button.button == SDL_BUTTON_LEFT &&
                state.showDebugPanel &&
                pointInsideRect(
                    event.button.x,
                    event.button.y,
                    map3dDebugDumpButtonRect()
                )
            ) {
                dumpRequested = true;
                continue;
            }

            if (event.type != SDL_EVENT_KEY_DOWN) {
                continue;
            }

            if (event.key.key == SDLK_ESCAPE) {
                running = false;
                continue;
            }

            switch (event.key.scancode) {
                case SDL_SCANCODE_1:
                    state.plane = 0;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_2:
                    state.plane = 1;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_3:
                    state.plane = 2;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_4:
                    state.plane = 3;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_LEFT:
                    state.yaw -= 0.08f;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_RIGHT:
                    state.yaw += 0.08f;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_UP:
                    state.pitch = std::clamp(
                        state.pitch + 0.05f,
                        0.12f,
                        1.35f
                    );
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_DOWN:
                    state.pitch = std::clamp(
                        state.pitch - 0.05f,
                        0.12f,
                        1.35f
                    );
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_EQUALS:
                case SDL_SCANCODE_KP_PLUS:
                    state.distance = std::max(
                        24.0f,
                        state.distance - 4.0f
                    );
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_MINUS:
                case SDL_SCANCODE_KP_MINUS:
                    state.distance = std::min(
                        180.0f,
                        state.distance + 4.0f
                    );
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_U:
                    state.showUnderlays = !state.showUnderlays;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_O:
                    state.showOverlays = !state.showOverlays;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_G:
                    state.showGrid = !state.showGrid;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_M:
                    state.showLocModels = !state.showLocModels;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_L:
                    state.showLocs = !state.showLocs;
                    titleDirty = true;
                    break;
                case SDL_SCANCODE_F3:
                    state.showDebugPanel = !state.showDebugPanel;
                    break;
                case SDL_SCANCODE_F9:
                    dumpRequested = true;
                    break;
                case SDL_SCANCODE_R:
                    state.yaw = 0.75f;
                    state.pitch = 0.62f;
                    state.distance = 82.0f;
                    titleDirty = true;
                    break;
                default:
                    break;
            }
        }

        frameTiming.inputMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - frameStart
            ).count();

        const auto fpsNow = FpsClock::now();
        const double fpsSampleSeconds =
            std::chrono::duration<double>(
                fpsNow - fpsSampleStart
            ).count();

        if (fpsSampleSeconds >= 0.5) {
            if (fpsSampleFrames > 0) {
                fps =
                    static_cast<double>(fpsSampleFrames) /
                    fpsSampleSeconds;
                frameTimeMs =
                    fpsSampleSeconds * 1000.0 /
                    static_cast<double>(fpsSampleFrames);
            }

            if (timingFrames > 0) {
                timingAverage =
                    averageFrameTimings(
                        timingSum,
                        timingFrames
                    );
            }

            timingSum = {};
            timingFrames = 0;
            fpsSampleFrames = 0;
            fpsSampleStart = fpsNow;
            titleDirty = true;
        }

        if (titleDirty) {
            std::ostringstream title;
            title
                << viewer3dTitle(*region, state)
                << " | "
                << std::fixed
                << std::setprecision(1)
                << fps << " FPS"
                << " | "
                << frameTimeMs << " ms";
            SDL_SetWindowTitle(
                window,
                title.str().c_str()
            );
            titleDirty = false;
        }

        const auto cameraStart = FpsClock::now();

        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        const Map3dPlaneStats stats =
            map3dPlaneStats(*centerTerrain, state.plane);
        const float targetY =
            stats.tiles == 0
                ? 0.0f
                : terrainWorldHeight(
                      static_cast<int>(
                          stats.heightSum /
                          static_cast<std::int64_t>(stats.tiles)
                      )
                  );
        const Vec3 target{0.0f, targetY, 0.0f};
        const float horizontalDistance =
            std::cos(state.pitch) * state.distance;
        const Vec3 camera{
            target.x +
                std::sin(state.yaw) * horizontalDistance,
            target.y +
                std::sin(state.pitch) * state.distance,
            target.z +
                std::cos(state.yaw) * horizontalDistance
        };

        const Vec3 forward =
            normalizeVec3(
                subtractVec3(target, camera)
            );
        const Vec3 right =
            normalizeVec3(
                crossVec3(
                    forward,
                    Vec3{0.0f, 1.0f, 0.0f}
                )
            );
        const Vec3 up =
            normalizeVec3(
                crossVec3(right, forward)
            );
        const ProjectionContext projection =
            makeProjectionContext(
                camera,
                right,
                up,
                forward,
                windowWidth,
                windowHeight
            );

        frameTiming.cameraMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - cameraStart
            ).count();

        const auto terrainStart = FpsClock::now();

        triangles.clear();

        const CachedTerrainPlane& cachedTerrain =
            terrainCache[state.plane];
        frameCounters.terrainTiles = cachedTerrain.tiles.size();

        for (const CachedTerrainTile& tile : cachedTerrain.tiles) {
            frameCounters.terrainVerticesProjected += tile.vertices.size();

            projectedScratch.clear();
            if (projectedScratch.capacity() < tile.vertices.size()) {
                projectedScratch.reserve(tile.vertices.size());
            }

            bool allVisible = true;
            for (Vec3 vertex : tile.vertices) {
                const ProjectedPoint point =
                    projectPoint(vertex, projection);
                projectedScratch.push_back(point);
                allVisible = allVisible && point.visible;
            }

            if (!allVisible) {
                continue;
            }

            for (const CachedTerrainTriangle& triangle : tile.triangles) {
                if (
                    triangle.surface ==
                        eld::graphics::map::TerrainSurface::Underlay &&
                    !state.showUnderlays
                ) {
                    continue;
                }
                if (
                    triangle.surface ==
                        eld::graphics::map::TerrainSurface::Overlay &&
                    !state.showOverlays
                ) {
                    continue;
                }

                const std::uint8_t ia = triangle.indices[0];
                const std::uint8_t ib = triangle.indices[1];
                const std::uint8_t ic = triangle.indices[2];
                if (
                    ia >= projectedScratch.size() ||
                    ib >= projectedScratch.size() ||
                    ic >= projectedScratch.size()
                ) {
                    continue;
                }

                const ProjectedPoint& a = projectedScratch[ia];
                const ProjectedPoint& b = projectedScratch[ib];
                const ProjectedPoint& c = projectedScratch[ic];

                ProjectedTriangle output;
                output.vertices = {
                    SDL_Vertex{
                        SDL_FPoint{a.x, a.y},
                        triangle.colors[0],
                        triangle.uvs[0]
                    },
                    SDL_Vertex{
                        SDL_FPoint{b.x, b.y},
                        triangle.colors[1],
                        triangle.uvs[1]
                    },
                    SDL_Vertex{
                        SDL_FPoint{c.x, c.y},
                        triangle.colors[2],
                        triangle.uvs[2]
                    }
                };
                output.texture = triangle.texture;
                output.depth = (a.depth + b.depth + c.depth) / 3.0f;
                triangles.push_back(output);
                ++frameCounters.terrainTriangles;
                if (triangle.texture != nullptr) {
                    ++frameCounters.terrainTexturedTriangles;
                }
            }
        }

        frameTiming.terrainMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - terrainStart
            ).count();

        const auto locModelsStart = FpsClock::now();
        Map3dLocRenderStats locRenderStats;

        if (state.showLocModels) {
            appendSceneLocModelTriangles(
                triangles,
                locSceneCache,
                locTextureCache,
                state.plane,
                projection,
                projectedScratch,
                &locRenderStats
            );
        }

        frameTiming.locModelsMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - locModelsStart
            ).count();

        frameCounters.locInstances = locRenderStats.instances;
        frameCounters.locInstancesCulled = locRenderStats.instancesCulled;
        frameCounters.locInstancesRendered = locRenderStats.instancesRendered;
        frameCounters.locParts = locRenderStats.parts;
        frameCounters.locMeshes = locRenderStats.meshes;
        frameCounters.locVerticesProjected = locRenderStats.verticesProjected;
        frameCounters.locTriangleCandidates = locRenderStats.triangleCandidates;
        frameCounters.locTriangles = locRenderStats.triangles;
        frameCounters.locTexturedTriangles = locRenderStats.texturedTriangles;
        frameCounters.totalTriangles = triangles.size();

        const auto sortStart = FpsClock::now();
        std::sort(
            triangles.begin(),
            triangles.end(),
            [](const ProjectedTriangle& a,
               const ProjectedTriangle& b) {
                return a.depth > b.depth;
            }
        );

        frameTiming.sortMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - sortStart
            ).count();

        const auto submitStart = FpsClock::now();

        SDL_SetRenderDrawColor(
            renderer,
            18,
            20,
            24,
            255
        );
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );

        // Keep the painter-sorted triangle order intact while switching SDL
        // textures only when the next triangle actually needs a different
        // source. Consecutive triangles with the same texture remain batched.
        geometry.clear();
        if (geometry.capacity() < triangles.size() * 3) {
            geometry.reserve(triangles.size() * 3);
        }

        SDL_Texture* currentTexture = nullptr;
        bool haveBatch = false;

        const auto flushGeometry = [&] {
            if (geometry.empty()) {
                return;
            }

            if (
                !SDL_RenderGeometry(
                    renderer,
                    currentTexture,
                    geometry.data(),
                    static_cast<int>(geometry.size()),
                    nullptr,
                    0
                )
            ) {
                throw std::runtime_error(
                    std::string(
                        "SDL_RenderGeometry failed: "
                    ) +
                    SDL_GetError()
                );
            }

            ++frameCounters.geometryBatches;
            frameCounters.geometryVertices += geometry.size();
            geometry.clear();
        };

        for (const ProjectedTriangle& triangle : triangles) {
            if (
                !haveBatch ||
                triangle.texture != currentTexture
            ) {
                flushGeometry();
                currentTexture = triangle.texture;
                haveBatch = true;
            }

            geometry.insert(
                geometry.end(),
                triangle.vertices.begin(),
                triangle.vertices.end()
            );
        }

        flushGeometry();

        frameTiming.submitMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - submitStart
            ).count();

        const auto debugDrawStart = FpsClock::now();

        if (state.showGrid) {
            SDL_SetRenderDrawColor(
                renderer,
                18,
                22,
                26,
                180
            );

            for (const ProjectedTriangle& triangle : triangles) {
                for (std::size_t edge = 0; edge < 3; ++edge) {
                    const SDL_FPoint& a =
                        triangle.vertices[edge].position;
                    const SDL_FPoint& b =
                        triangle.vertices[(edge + 1) % 3].position;
                    SDL_RenderLine(
                        renderer,
                        a.x,
                        a.y,
                        b.x,
                        b.y
                    );
                }
            }
        }

        if (state.showLocs) {
            for (const auto& loc : sceneLocs) {
                if (loc.scenePlane != state.plane) {
                    continue;
                }

                drawSceneLocDebug(
                    renderer,
                    loc,
                    camera,
                    right,
                    up,
                    forward,
                    windowWidth,
                    windowHeight
                );
            }
        }

        frameTiming.debugDrawMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - debugDrawStart
            ).count();

        const auto panelStart = FpsClock::now();
        if (state.showDebugPanel) {
            drawMap3dDebugPanel(
                renderer,
                state,
                fps,
                frameTimeMs,
                timingAverage,
                frameCounters,
                dumpStatus
            );
        }
        frameTiming.panelMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - panelStart
            ).count();

        const auto presentStart = FpsClock::now();
        SDL_RenderPresent(renderer);
        frameTiming.presentMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - presentStart
            ).count();

        frameTiming.cpuFrameMs =
            std::chrono::duration<double, std::milli>(
                FpsClock::now() - frameStart
            ).count();

        lastTiming = frameTiming;
        lastCounters = frameCounters;
        addFrameTimings(timingSum, frameTiming);
        ++timingFrames;
        ++fpsSampleFrames;

        if (dumpRequested) {
            writeDebugReport(
                windowWidth,
                windowHeight
            );
            dumpRequested = false;
        }

        SDL_Delay(8);
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
        << "  " << executable
        << " <cache-root> view <region-id>\n"
        << "  " << executable
        << " <cache-root> view <region-x> <region-y>\n"
        << "  " << executable
        << " <cache-root> view3d <region-id>\n"
        << "  " << executable
        << " <cache-root> view3d <region-x> <region-y>\n"
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
        << " cache validate\n"
        << "  " << executable
        << " cache view 7499\n"
        << "  " << executable
        << " cache view3d 7499\n";
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


        if (command == "view3d") {
            if (
                argc != 4 &&
                argc != 5
            ) {
                printUsage(argv[0]);
                return 1;
            }

            if (!mapIndex.error.empty()) {
                std::cerr
                    << "Cannot view a region in 3D: "
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

            viewRegion3d(
                cache,
                maps,
                mapIndex.entries,
                regionId
            );
            return 0;
        }

        if (command == "view") {
            if (
                argc != 4 &&
                argc != 5
            ) {
                printUsage(argv[0]);
                return 1;
            }

            if (!mapIndex.error.empty()) {
                std::cerr
                    << "Cannot view a region: "
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

            viewRegion(
                cache,
                maps,
                mapIndex.entries,
                regionId
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
