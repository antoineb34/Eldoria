#pragma once

#include "graphics/model/ModelHandle.h"
#include "Transform.h"

namespace eld::render {

struct RenderObject {
    eld::graphics::ModelHandle model;
    Transform transform;
    bool visible = true;
};

}
