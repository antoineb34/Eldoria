#pragma once

#include <vector>

#include "model/ModelAsset.h"
#include "../software/camera/Projection.h"

namespace eld::render {

struct RenderVertex {
    ScreenPoint screen;
};

struct RenderFace {
    int faceIndex = -1;
    const eld::model::Face* source = nullptr;

    int a = 0;
    int b = 0;
    int c = 0;

    float depthAvg = 0.0f;
    float depthMin = 0.0f;
    float depthMax = 0.0f;
};

struct RenderMesh {
    std::vector<RenderVertex> vertices;
    std::vector<RenderFace> faces;
};

}
