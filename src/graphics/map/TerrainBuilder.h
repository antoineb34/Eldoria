#pragma once

#include <cstddef>

#include "floor/FloorLoader.h"
#include "render/model/ModelResource.h"
#include "texture/TextureSystem.h"
#include "world/Terrain.h"

namespace eld::graphics {

class TerrainBuilder {
public:
    // scenePlane is the effective World/scene plane.
    //
    // The builder internally selects every source terrain
    // layer that projects onto it.
    eld::render::ModelResource build(
        const eld::world::Terrain& terrain,
        std::size_t scenePlane,
        const eld::floor::FloorLoader& floors,
        TextureSystem& textures
    ) const;
};

}
