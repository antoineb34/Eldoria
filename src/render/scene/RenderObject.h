#pragma once

#include "../model/RenderModel.h"
#include "Transform.h"

namespace eld::render {

struct RenderObject {
    const RenderModel* model = nullptr;
    Transform transform;
};

}
