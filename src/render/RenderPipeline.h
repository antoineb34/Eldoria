#pragma once

#include "backend/IRenderBackend.h"
#include "pipeline/RenderQueueBuilder.h"
#include "scene/RenderScene.h"

namespace eld::render {

class RenderPipeline {
public:
    void render(
        const RenderScene& scene,
        IRenderBackend& backend
    );

private:
    RenderQueueBuilder queueBuilder_;
};

}
