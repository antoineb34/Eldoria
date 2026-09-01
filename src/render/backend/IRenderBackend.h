#pragma once

#include "camera/Camera.h"
#include "graphics/GraphicsResources.h"
#include "graphics/model/ModelHandle.h"
#include "scene/Transform.h"

namespace eld::render {

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    virtual void beginFrame(
        const Camera& camera
    ) = 0;

    virtual void draw(
        eld::graphics::ModelHandle model,
        const Transform& transform,
        const eld::graphics::GraphicsResources& resources
    ) = 0;

    virtual void endFrame() = 0;
};

}
