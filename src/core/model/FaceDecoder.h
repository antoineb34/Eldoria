#pragma once

#include <cstdint>
#include <vector>

#include "ModelFooter.h"
#include "ModelLayout.h"

namespace rf::model {

struct TextureTriangle {
    int a = 0;
    int b = 0;
    int c = 0;
};

struct Face {
    int a = 0;
    int b = 0;
    int c = 0;

    uint16_t color = 0;

    uint8_t priority = 0;
    uint8_t alpha = 0;

    uint8_t triangleType = 0;
    uint8_t renderType = 0;

    int textureInfo = -1;
    int textureTriangleIndex = -1;
};

int findMatchingTextureTriangle(
    const Face& face,
    const std::vector<TextureTriangle>& textureTriangles
);

std::vector<Face> decodeFaces(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

std::vector<TextureTriangle> decodeTextureTriangles(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
