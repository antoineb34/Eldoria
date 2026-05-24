#pragma once

#include <cstdint>
#include <vector>

namespace rf::model {

struct ModelFooter {

    uint32_t vertexCount;
    uint32_t triangleCount;

    uint32_t textureTriangleCount;

    uint32_t textureFlag;
    uint32_t priorityFlag;
    uint32_t alphaFlag;

    uint32_t triangleSkinFlag;
    uint32_t vertexSkinFlag;

    uint32_t xDataLength;
    uint32_t yDataLength;
    uint32_t zDataLength;

    uint32_t triangleDataLength;
};

ModelFooter readModelFooter(
    const std::vector<char>& payload
);

}
