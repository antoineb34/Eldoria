#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../texture/TextureAsset.h"

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

    int texturePointer = -1;
    int textureUVMappingIndex = -1;
};

struct TextureUVMapping {
    int originVertex = 0;
    int uVertex = 0;
    int vVertex = 0;
};

struct ModelAsset {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<TextureUVMapping> textureUVMappings;

    std::unordered_map<int, rf::texture::TextureAsset> textures;
};

}
