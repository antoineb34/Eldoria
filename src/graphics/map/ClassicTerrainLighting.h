#pragma once

#include <cstddef>

#include "floor/FloorLoader.h"
#include "math/Vec4.h"
#include "world/Terrain.h"

namespace eld::graphics::terrain
{

struct ClassicCornerShades
{
    int southwest = 0;
    int southeast = 0;
    int northeast = 0;
    int northwest = 0;
};

struct ClassicSurfaceLighting
{
    ClassicCornerShades shades{};
    bool visible = false;
};

struct ClassicTileLighting
{
    ClassicSurfaceLighting underlay{};
    ClassicSurfaceLighting overlay{};
};

ClassicTileLighting buildClassicTerrainLighting(
    const eld::world::Terrain& terrain,
    const eld::world::TerrainLayerPosition& position,
    const eld::floor::FloorLoader& floors
);

int classicShadeForPoint(
    int pointType,
    const ClassicCornerShades& shades
);

eld::math::Vec4 classicTerrainColor(
    int shade,
    bool textured
);

}
