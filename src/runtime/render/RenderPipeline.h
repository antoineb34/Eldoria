#pragma once

#include "backend/RenderBackend.h"
#include "model/ModelManager.h"
#include "scene/RenderScene.h"
#include "texture/TextureManager.h"

namespace eld::render {

class RenderPipeline {
public:
    void render(
        const RenderScene& scene,
        const ModelManager& models,
        const TextureManager& textures,
        RenderBackend& backend
    ) const;
};

}
