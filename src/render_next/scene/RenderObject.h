#pragma once

#include "../../core/assets/model/ModelAsset.h"

namespace rf::render_next {

struct RenderObject {
    const rf::model::ModelAsset* model = nullptr;

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;

    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;

    float scale = 1.0f;
};

}
