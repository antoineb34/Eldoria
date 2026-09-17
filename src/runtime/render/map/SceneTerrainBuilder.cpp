#include "SceneTerrainBuilder.h"

#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace eld::render::map {
namespace {

const std::vector<std::vector<int>>& pointPatterns() {
    static const std::vector<std::vector<int>> Patterns{
        {1, 3, 5, 7},
        {1, 3, 5, 7},
        {1, 3, 5, 7},
        {1, 3, 5, 7, 6},
        {1, 3, 5, 7, 6},
        {1, 3, 5, 7, 6},
        {1, 3, 5, 7, 6},
        {1, 3, 5, 7, 2, 6},
        {1, 3, 5, 7, 2, 8},
        {1, 3, 5, 7, 2, 8},
        {1, 3, 5, 7, 11, 12},
        {1, 3, 5, 7, 11, 12},
        {1, 3, 5, 7, 13, 14}
    };
    return Patterns;
}

const std::vector<std::vector<int>>& elementPatterns() {
    static const std::vector<std::vector<int>> Patterns{
        {0,1,2,3, 0,0,1,3},
        {1,1,2,3, 1,0,1,3},
        {0,1,2,3, 1,0,1,3},
        {0,0,1,2, 0,0,2,4, 1,0,4,3},
        {0,0,1,4, 0,0,4,3, 1,1,2,4},
        {0,0,4,3, 1,0,1,2, 1,0,2,4},
        {0,1,2,4, 1,0,1,4, 1,0,4,3},
        {0,4,1,2, 0,4,2,5, 1,0,4,5, 1,0,5,3},
        {0,4,1,2, 0,4,2,3, 0,4,3,5, 1,0,4,5},
        {0,0,4,5, 1,4,1,2, 1,4,2,3, 1,4,3,5},
        {0,0,1,5, 0,1,4,5, 0,1,2,4, 1,0,5,3, 1,5,4,3, 1,4,2,3},
        {1,0,1,5, 1,1,4,5, 1,1,2,4, 0,0,5,3, 0,5,4,3, 0,4,2,3},
        {1,0,5,4, 1,0,1,5, 0,0,4,3, 0,4,5,3, 0,5,2,3, 0,1,2,5}
    };
    return Patterns;
}


std::array<SceneTerrainUv, 3> sharedTileTextureUvs(
    const SceneTileMesh& mesh,
    const std::array<std::uint8_t, 3>& indices
) {
    if (mesh.vertices.size() < 4) {
        throw std::runtime_error(
            "RuneScape shaped tile is missing corner vertices"
        );
    }

    const SceneTerrainVertex& origin = mesh.vertices[0];
    const SceneTerrainVertex& uPoint = mesh.vertices[1];
    const SceneTerrainVertex& vPoint = mesh.vertices[3];

    const double ux = static_cast<double>(uPoint.x - origin.x);
    const double uz = static_cast<double>(uPoint.z - origin.z);
    const double vx = static_cast<double>(vPoint.x - origin.x);
    const double vz = static_cast<double>(vPoint.z - origin.z);

    const double uu = ux * ux + uz * uz;
    const double uv = ux * vx + uz * vz;
    const double vv = vx * vx + vz * vz;
    const double denominator = uu * vv - uv * uv;

    if (std::abs(denominator) <= 0.000001) {
        throw std::runtime_error(
            "RuneScape shaped-tile texture basis is degenerate"
        );
    }

    std::array<SceneTerrainUv, 3> result{};

    for (std::size_t corner = 0; corner < 3; ++corner) {
        const SceneTerrainVertex& point =
            mesh.vertices[indices[corner]];

        const double rx = static_cast<double>(point.x - origin.x);
        const double rz = static_cast<double>(point.z - origin.z);
        const double ru = rx * ux + rz * uz;
        const double rv = rx * vx + rz * vz;

        result[corner] = SceneTerrainUv{
            static_cast<float>((vv * ru - uv * rv) / denominator),
            static_cast<float>((uu * rv - uv * ru) / denominator)
        };
    }

    return result;
}

std::array<SceneTerrainUv, 3> triangleTextureUvs() {
    // The non-flat classic TileOverlay path uses the rendered triangle's
    // own A/B/C vertices as the texture-mapping basis. In an explicit-UV
    // renderer that maps directly to the canonical texture triangle.
    return {
        SceneTerrainUv{0.0f, 0.0f},
        SceneTerrainUv{1.0f, 0.0f},
        SceneTerrainUv{0.0f, 1.0f}
    };
}

int rotateVertexType(
    int type,
    int rotation
) {
    if ((type & 1) == 0 && type <= 8) {
        return ((type - rotation * 2 - 1) & 7) + 1;
    }

    if (type > 8 && type <= 12) {
        return ((type - 9 - rotation) & 3) + 9;
    }

    if (type > 12 && type <= 16) {
        return ((type - 13 - rotation) & 3) + 13;
    }

    return type;
}

SceneTerrainVertex vertexForType(
    int type,
    int baseX,
    int baseZ,
    const TerrainCornerHeights& h,
    const TerrainTileShades& shades
) {
    constexpr int Half = SceneTerrainBuilder::TileSize / 2;
    constexpr int Quarter = SceneTerrainBuilder::TileSize / 4;
    constexpr int ThreeQuarter =
        SceneTerrainBuilder::TileSize * 3 / 4;

    const auto midpoint = [](int a, int b) {
        return (a + b) >> 1;
    };

    const auto make = [&](
        int x,
        int y,
        int z,
        int underlay,
        int overlay
    ) {
        return SceneTerrainVertex{
            x,
            y,
            z,
            underlay,
            overlay
        };
    };

    switch (type) {
        case 1:
            return make(
                baseX,
                h.southwest,
                baseZ,
                shades.underlay.southwest,
                shades.overlay.southwest
            );
        case 2:
            return make(
                baseX + Half,
                midpoint(h.southwest, h.southeast),
                baseZ,
                midpoint(
                    shades.underlay.southwest,
                    shades.underlay.southeast
                ),
                midpoint(
                    shades.overlay.southwest,
                    shades.overlay.southeast
                )
            );
        case 3:
            return make(
                baseX + SceneTerrainBuilder::TileSize,
                h.southeast,
                baseZ,
                shades.underlay.southeast,
                shades.overlay.southeast
            );
        case 4:
            return make(
                baseX + SceneTerrainBuilder::TileSize,
                midpoint(h.southeast, h.northeast),
                baseZ + Half,
                midpoint(
                    shades.underlay.southeast,
                    shades.underlay.northeast
                ),
                midpoint(
                    shades.overlay.southeast,
                    shades.overlay.northeast
                )
            );
        case 5:
            return make(
                baseX + SceneTerrainBuilder::TileSize,
                h.northeast,
                baseZ + SceneTerrainBuilder::TileSize,
                shades.underlay.northeast,
                shades.overlay.northeast
            );
        case 6:
            return make(
                baseX + Half,
                midpoint(h.northeast, h.northwest),
                baseZ + SceneTerrainBuilder::TileSize,
                midpoint(
                    shades.underlay.northeast,
                    shades.underlay.northwest
                ),
                midpoint(
                    shades.overlay.northeast,
                    shades.overlay.northwest
                )
            );
        case 7:
            return make(
                baseX,
                h.northwest,
                baseZ + SceneTerrainBuilder::TileSize,
                shades.underlay.northwest,
                shades.overlay.northwest
            );
        case 8:
            return make(
                baseX,
                midpoint(h.northwest, h.southwest),
                baseZ + Half,
                midpoint(
                    shades.underlay.northwest,
                    shades.underlay.southwest
                ),
                midpoint(
                    shades.overlay.northwest,
                    shades.overlay.southwest
                )
            );
        case 9:
            return make(
                baseX + Half,
                midpoint(h.southwest, h.southeast),
                baseZ + Quarter,
                midpoint(
                    shades.underlay.southwest,
                    shades.underlay.southeast
                ),
                midpoint(
                    shades.overlay.southwest,
                    shades.overlay.southeast
                )
            );
        case 10:
            return make(
                baseX + ThreeQuarter,
                midpoint(h.southeast, h.northeast),
                baseZ + Half,
                midpoint(
                    shades.underlay.southeast,
                    shades.underlay.northeast
                ),
                midpoint(
                    shades.overlay.southeast,
                    shades.overlay.northeast
                )
            );
        case 11:
            return make(
                baseX + Half,
                midpoint(h.northeast, h.northwest),
                baseZ + ThreeQuarter,
                midpoint(
                    shades.underlay.northeast,
                    shades.underlay.northwest
                ),
                midpoint(
                    shades.overlay.northeast,
                    shades.overlay.northwest
                )
            );
        case 12:
            return make(
                baseX + Quarter,
                midpoint(h.northwest, h.southwest),
                baseZ + Half,
                midpoint(
                    shades.underlay.northwest,
                    shades.underlay.southwest
                ),
                midpoint(
                    shades.overlay.northwest,
                    shades.overlay.southwest
                )
            );
        case 13:
            return make(
                baseX + Quarter,
                h.southwest,
                baseZ + Quarter,
                shades.underlay.southwest,
                shades.overlay.southwest
            );
        case 14:
            return make(
                baseX + ThreeQuarter,
                h.southeast,
                baseZ + Quarter,
                shades.underlay.southeast,
                shades.overlay.southeast
            );
        case 15:
            return make(
                baseX + ThreeQuarter,
                h.northeast,
                baseZ + ThreeQuarter,
                shades.underlay.northeast,
                shades.overlay.northeast
            );
        case 16:
            return make(
                baseX + Quarter,
                h.northwest,
                baseZ + ThreeQuarter,
                shades.underlay.northwest,
                shades.overlay.northwest
            );
        default:
            throw std::runtime_error(
                "invalid RuneScape shaped-tile vertex type"
            );
    }
}

}

SceneTileMesh SceneTerrainBuilder::buildTile(
    int tileX,
    int tileZ,
    std::uint8_t sceneShape,
    std::uint8_t rotation,
    const TerrainCornerHeights& heights,
    const TerrainTileShades& shades
) const {
    if (sceneShape >= SceneShapeCount) {
        throw std::runtime_error(
            "RuneScape scene tile shape must be 0..12"
        );
    }

    rotation &= 3u;

    // Scene shape 1 is represented by the classic TileUnderlay path. Its
    // angle argument is ignored by the original scene implementation, so a
    // full-tile overlay must keep the fixed SW/SE/NE/NW diagonal.
    const std::uint8_t effectiveRotation =
        sceneShape <= 1 ? 0u : rotation;

    const auto& pointPattern =
        pointPatterns()[sceneShape];
    const auto& elementPattern =
        elementPatterns()[sceneShape];

    SceneTileMesh mesh;
    mesh.vertices.reserve(pointPattern.size());
    mesh.triangles.reserve(elementPattern.size() / 4);

    const int baseX = tileX * TileSize;
    const int baseZ = tileZ * TileSize;

    for (int rawType : pointPattern) {
        mesh.vertices.push_back(
            vertexForType(
                rotateVertexType(
                    rawType,
                    static_cast<int>(effectiveRotation)
                ),
                baseX,
                baseZ,
                heights,
                shades
            )
        );
    }

    for (
        std::size_t offset = 0;
        offset < elementPattern.size();
        offset += 4
    ) {
        const int layer = elementPattern[offset];
        std::array<std::uint8_t, 3> indices{};

        for (std::size_t i = 0; i < 3; ++i) {
            int index = elementPattern[offset + 1 + i];

            if (index < 4) {
                index =
                    (index - static_cast<int>(effectiveRotation)) & 3;
            }

            if (
                index < 0 ||
                index >= static_cast<int>(mesh.vertices.size())
            ) {
                throw std::runtime_error(
                    "RuneScape shaped-tile triangle index is invalid"
                );
            }

            indices[i] = static_cast<std::uint8_t>(index);
        }

        const TerrainSurface surface =
            layer == 0
                ? TerrainSurface::Underlay
                : TerrainSurface::Overlay;

        std::array<SceneTerrainUv, 3> textureUvs{};

        if (surface == TerrainSurface::Overlay) {
            const bool flat =
                heights.southwest == heights.southeast &&
                heights.southwest == heights.northeast &&
                heights.southwest == heights.northwest;

            if (sceneShape == 1 || flat) {
                // TileUnderlay and flat TileOverlay rendering share one
                // texture basis across the whole tile. For rotated shaped
                // overlays vertices 0,1,3 are themselves rotated, exactly as
                // in the original scene object, so this also rotates texture
                // orientation with the overlay shape.
                textureUvs = sharedTileTextureUvs(mesh, indices);
            }
            else {
                textureUvs = triangleTextureUvs();
            }
        }

        mesh.triangles.push_back({
            indices,
            surface,
            textureUvs
        });
    }

    return mesh;
}

}
