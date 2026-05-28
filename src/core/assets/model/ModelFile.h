#pragma once

#include <cstdint>
#include <vector>

namespace rf::model {

struct ModelFooter {

    uint32_t vertexCount = 0;
    uint32_t triangleCount = 0;

    uint32_t textureTriangleCount = 0;

    uint32_t textureFlag = 0;
    uint32_t priorityFlag = 0;
    uint32_t alphaFlag = 0;

    uint32_t triangleSkinFlag = 0;
    uint32_t vertexSkinFlag = 0;

    uint32_t xDataLength = 0;
    uint32_t yDataLength = 0;
    uint32_t zDataLength = 0;

    uint32_t triangleDataLength = 0;
};

struct ModelLayout {

    int vertexFlagsOffset = 0;

    int triangleTypesOffset = 0;

    int trianglePrioritiesOffset = 0;
    int triangleSkinsOffset = 0;

    int texturePointersOffset = 0;

    int vertexSkinsOffset = 0;

    int triangleAlphasOffset = 0;

    int triangleDataOffset = 0;

    int triangleColorsOffset = 0;

    int textureDataOffset = 0;

    int xDataOffset = 0;
    int yDataOffset = 0;
    int zDataOffset = 0;
};

struct ModelFile {

    std::vector<uint8_t> payload;

    ModelFooter footer;

    ModelLayout layout;
};

}
