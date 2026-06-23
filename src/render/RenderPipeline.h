#pragma once

#include "backend/IRenderBackend.h"
#include "graphics/GraphicsResources.h"
#include "scene/RenderScene.h"

namespace eld::render {

class RenderPipeline {
public:
    void render(
        const RenderScene& scene,
        const eld::graphics::GraphicsResources& resources,
        IRenderBackend& backend
    ) const;
};

}
