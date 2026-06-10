#pragma once

#include <vector>

#include "RenderCamera.h"
#include "RenderObject.h"

namespace eld::render_next {

struct RenderScene {
    RenderCamera camera;
    std::vector<RenderObject> objects;
};

}
