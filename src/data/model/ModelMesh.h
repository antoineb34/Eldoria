#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace eld::model {

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    std::optional<std::uint8_t> skin;
};

struct Face {
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    std::uint32_t c = 0;

    std::uint16_t color = 0;

    std::uint8_t priority = 0;
    std::uint8_t alpha = 0;
    std::uint8_t renderType = 0;

    std::optional<std::uint8_t> skin;

    std::optional<std::uint16_t> textureId;
    std::optional<std::uint32_t> textureMappingIndex;
};

struct TextureMapping {
    std::uint32_t originVertex = 0;
    std::uint32_t uVertex = 0;
    std::uint32_t vVertex = 0;
};

struct ModelMesh {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<TextureMapping> textureMappings;
};

}
