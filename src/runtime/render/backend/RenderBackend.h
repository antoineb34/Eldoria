#pragma once

#include "camera/Camera.h"
#include "model/ModelHandle.h"
#include "model/ModelResource.h"
#include "scene/Transform.h"
#include "texture/TextureManager.h"

namespace eld::render {

class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual void beginFrame(
        const Camera& camera
    ) = 0;

    virtual void draw(
        ModelHandle handle,
        const ModelResource& model,
        const Transform& transform,
        const TextureManager& textures
    ) = 0;

    virtual void endFrame() = 0;
};

}
