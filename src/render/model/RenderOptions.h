#pragma once

namespace rf::render {

    struct RenderOptions {
        bool fillTriangles = true;
        bool showWireframe = true;
        bool showVertices = false;
        bool useAlpha = true;
        bool highlightTexturedFaces = false;
    };

    struct ModelTransform {
        float rotationX = 0.0f;
        float rotationY = 0.0f;
        float rotationZ = 0.0f;
        float scale = 1.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float offsetZ = 0.0f;
    };

}
