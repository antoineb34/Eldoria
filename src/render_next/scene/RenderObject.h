#pragma once

#include "model/ModelAsset.h"
#include "Transform.h"

namespace eld::render_next {

struct RenderObject {
    const eld::model::ModelAsset* model = nullptr;
    Transform transform;
};

}
