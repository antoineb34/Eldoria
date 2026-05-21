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

    int textureFlag = -1;
    int textureTriangleIndex = -1;
};

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
