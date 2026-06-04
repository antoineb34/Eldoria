#pragma once

#include <vector>

#include "RenderCamera.h"
#include "RenderObject.h"

namespace rf::render_next {

struct RenderScene {
    RenderCamera camera;
    std::vector<RenderObject> objects;
};

}
