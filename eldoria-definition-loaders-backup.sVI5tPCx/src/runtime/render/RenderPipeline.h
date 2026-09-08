#pragma once

#include "backend/RenderBackend.h"
#include "render/GraphicsResources.h"
#include "scene/RenderScene.h"

namespace eld::render {

class RenderPipeline {
public:
    void render(
        const RenderScene& scene,
        const eld::render::GraphicsResources& resources,
        RenderBackend& backend
    ) const;
};

}
