#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

#include "SceneTerrainBuilder.h"
#include "definition/floor/FloorRepository.h"
#include "map/MapTile.h"

namespace eld::render::map {

using TerrainTileSampler =
    std::function<
        const eld::map::MapTile*(
            std::size_t plane,
            int x,
            int y
        )
    >;

struct SceneTileAppearance {
    TerrainTileShades shades{};

    bool underlayVisible = false;
    bool overlayVisible = false;

    std::optional<std::uint8_t> textureId;
};

class ClassicTerrainAppearanceBuilder {
public:
    SceneTileAppearance build(
        std::size_t plane,
        int tileX,
        int tileY,
        const TerrainTileSampler& sample,
        const eld::definition::FloorRepository& floors
    ) const;

    static std::uint32_t paletteRgb(
        int paletteIndex
    );

    // Convert the classic textured-floor light scalar into an RGB
    // modulation value for renderers whose texture path uses vertex color.
    // This keeps the cache texture itself intact while preserving the terrain
    // light already calculated by this compatibility builder.
    static std::uint32_t textureModulationRgb(
        int textureShade
    );

private:
    static int vertexLight(
        std::size_t plane,
        int x,
        int y,
        const TerrainTileSampler& sample
    );
};

}
