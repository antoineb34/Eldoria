#pragma once

#include "../order/FaceOrderMode.h"

namespace rf::render {

struct RenderOptions {
    bool fillTriangles = true;
    bool showWireframe = true;
    bool showVertices = false;
    bool useAlpha = true;
    bool highlightTexturedFaces = false;

    FaceOrderMode faceOrderMode = FaceOrderMode::RuneScapePriority;
};

}
