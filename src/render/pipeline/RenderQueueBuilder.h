#pragma once

#include "RenderQueue.h"
#include "../scene/RenderScene.h"

namespace eld::render {

class RenderQueueBuilder {
public:
    RenderQueue build(
        const RenderScene& scene
    ) const;
};

}
