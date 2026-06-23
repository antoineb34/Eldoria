#pragma once

#include <vector>

#include "camera/Camera.h"
#include "RenderObject.h"

namespace eld::render {

struct RenderScene {
    Camera camera;
    std::vector<RenderObject> objects;
};

}
