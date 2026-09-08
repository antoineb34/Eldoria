#pragma once

#include "render/model/ModelHandle.h"
#include "Transform.h"

namespace eld::render {

struct RenderObject {
    eld::render::ModelHandle model;
    Transform transform;
    bool visible = true;
};

}
