#pragma once

#include <vector>
#include "ModelFooter.h"
#include "ModelLayout.h"

namespace rf::model {

struct Vertex {
    int x = 0;
    int y = 0;
    int z = 0;
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

struct TextureTriangle {
    int a = 0;
    int b = 0;
    int c = 0;
};

struct ModelDef {
    ModelFooter footer;
    ModelLayout layout;

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<TextureTriangle> textureTriangles;
};

}
