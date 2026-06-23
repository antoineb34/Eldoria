#pragma once

#include "../pipeline/RenderItem.h"
#include "../scene/RenderCamera.h"

namespace eld::render {

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    virtual void beginFrame(
        const RenderCamera& camera
    ) = 0;

    virtual void draw(
        const RenderItem& item
    ) = 0;

    virtual void endFrame() = 0;
};

}
