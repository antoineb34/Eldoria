#pragma once

#include "model/ModelAsset.h"
#include "Transform.h"

namespace eld::render {

struct RenderObject {
    const eld::model::ModelAsset* model = nullptr;
    Transform transform;
};

}
