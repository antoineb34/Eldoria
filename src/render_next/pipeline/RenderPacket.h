#pragma once

#include <cstdint>

namespace eld::render_next {

struct RenderPacket {
    int objectIndex = -1;
    int faceIndex = -1;

    int a = 0;
    int b = 0;
    int c = 0;

    float depthAvg = 0.0f;
    float depthMin = 0.0f;
    float depthMax = 0.0f;

    uint16_t color = 0;
    uint8_t alpha = 0;
    uint8_t renderType = 0;
    uint8_t priority = 0;

    int texturePointer = -1;
    int textureUVMappingIndex = -1;

    bool visible = true;
};

}
