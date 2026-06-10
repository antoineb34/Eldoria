#pragma once

namespace eld::render {

struct RenderOptions {
    bool fillTriangles = true;
    bool showWireframe = true;
    bool showVertices = false;
    bool useAlpha = true;
    bool highlightTexturedFaces = false;
};

}
