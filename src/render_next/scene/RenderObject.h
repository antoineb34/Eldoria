#pragma once

#include "model/ModelAsset.h"
#include "Transform.h"

namespace rf::render_next {

struct RenderObject {
    const rf::model::ModelAsset* model = nullptr;
    Transform transform;
};

}
