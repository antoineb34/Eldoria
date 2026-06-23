#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "../texture/TextureHandle.h"

namespace eld::graphics {

struct RenderColor {
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;
    float alpha = 1.0f;
};

struct RenderVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float normalX = 0.0f;
    float normalY = 0.0f;
    float normalZ = 1.0f;

    float u = 0.0f;
    float v = 0.0f;

    RenderColor color;
};

enum class AlphaMode : std::uint8_t {
    Opaque,
    Masked,
    Blended
};

struct RenderMaterial {
    RenderColor baseColor;

    std::optional<TextureHandle> texture;

    AlphaMode alphaMode =
        AlphaMode::Opaque;

    bool doubleSided = false;
};

struct RenderMeshSection {
    std::uint32_t firstIndex = 0;
    std::uint32_t indexCount = 0;
    std::uint32_t materialIndex = 0;

    int sortOrder = 0;
};

struct RenderMesh {
    std::vector<RenderVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<RenderMeshSection> sections;
};

struct RenderModel {
    std::vector<RenderMesh> meshes;
    std::vector<RenderMaterial> materials;
};

}
