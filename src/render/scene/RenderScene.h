#pragma once

#include <vector>

#include "RenderCamera.h"
#include "RenderObject.h"

namespace eld::render {

struct RenderScene {
    RenderCamera camera;
    std::vector<RenderObject> objects;
};

}
