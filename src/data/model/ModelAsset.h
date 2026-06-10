#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../../core/assets/texture/TextureAsset.h"

namespace rf::model {

    struct Vertex {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
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
