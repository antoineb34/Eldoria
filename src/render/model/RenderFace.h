#pragma once

#include "../../core/assets/model/ModelAsset.h"

#include "../software/camera/Projection.h"

namespace rf::render {

struct RenderFace {

    const rf::model::Face* face = nullptr;

    ScreenPoint a;
    ScreenPoint b;
    ScreenPoint c;

    float depth = 0.0f;
};

}
