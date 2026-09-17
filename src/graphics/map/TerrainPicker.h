#pragma once

#include <optional>

#include "render/camera/Camera.h"
#include "world/Terrain.h"
#include "world/World.h"


namespace eld::graphics
{

    class TerrainPicker
    {
    public:
        std::optional<
            eld::world::TilePosition
        >
        pick(
            const eld::world::World& world,
            const eld::render::Camera& camera,
            const eld::world::TerrainOrigin& sceneOrigin,
            float mouseX,
            float mouseY
        ) const;
    };

}
