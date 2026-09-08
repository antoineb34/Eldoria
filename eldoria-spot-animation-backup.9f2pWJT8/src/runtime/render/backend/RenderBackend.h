#pragma once

#include "camera/Camera.h"
#include "render/GraphicsResources.h"
#include "render/model/ModelHandle.h"
#include "scene/Transform.h"

namespace eld::render {

class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    virtual void beginFrame(
        const Camera& camera
    ) = 0;

    virtual void draw(
        eld::render::ModelHandle model,
        const Transform& transform,
        const eld::render::GraphicsResources& resources
    ) = 0;

    virtual void endFrame() = 0;
};

}
