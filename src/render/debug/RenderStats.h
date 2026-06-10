#pragma once

namespace eld::render {

struct RenderStats {
    int objects = 0;
    int verticesInput = 0;
    int facesInput = 0;

    int verticesProjected = 0;
    int facesAssembled = 0;
    int facesCulled = 0;
    int facesDrawn = 0;
};

}
