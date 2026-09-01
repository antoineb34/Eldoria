#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace eld::graphics::map {

enum class TerrainSurface : std::uint8_t {
    Underlay = 0,
    Overlay = 1
};

struct TerrainCornerHeights {
    int southwest = 0;
    int southeast = 0;
    int northeast = 0;
    int northwest = 0;
};

struct TerrainCornerShades {
    int southwest = 12345678;
    int southeast = 12345678;
    int northeast = 12345678;
    int northwest = 12345678;
};

struct TerrainTileShades {
    TerrainCornerShades underlay{};
    TerrainCornerShades overlay{};
};

struct SceneTerrainVertex {
    int x = 0;
    int y = 0;
    int z = 0;

    // Classic raster values. For ordinary floors these are indices into the
    // 65,536-entry HSL palette. For textured overlays they are the classic
    // 0..127 texture-light scalars.
    int underlayShade = 12345678;
    int overlayShade = 12345678;
};

struct SceneTerrainUv {
    float u = 0.0f;
    float v = 0.0f;
};

struct SceneTerrainTriangle {
    std::array<std::uint8_t, 3> indices{};
    TerrainSurface surface = TerrainSurface::Underlay;

    // Explicit texture coordinates are stored per triangle rather than per
    // scene vertex because the classic client changes its texture-mapping
    // basis for sloped shaped tiles. The same scene vertex can therefore
    // legitimately have different texture coordinates in adjacent faces.
    std::array<SceneTerrainUv, 3> textureUvs{};
};

struct SceneTileMesh {
    std::vector<SceneTerrainVertex> vertices;
    std::vector<SceneTerrainTriangle> triangles;
};

class SceneTerrainBuilder {
public:
    static constexpr int TileSize = 128;
    static constexpr std::uint8_t SceneShapeCount = 13;

    SceneTileMesh buildTile(
        int tileX,
        int tileZ,
        std::uint8_t sceneShape,
        std::uint8_t rotation,
        const TerrainCornerHeights& heights,
        const TerrainTileShades& shades = {}
    ) const;
};

}
