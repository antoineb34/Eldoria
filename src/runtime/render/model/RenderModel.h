#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"
#include "texture/SamplerState.h"
#include "texture/TextureHandle.h"

namespace eld::render {

enum class AlphaMode : std::uint8_t {
    Opaque,
    Masked,
    Blended
};

struct RenderVertex {
    eld::math::Vec3 position;
    eld::math::Vec3 normal;

    eld::math::Vec2 uv;

    eld::math::Vec4 color{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };
};

struct RenderMaterial {
    eld::math::Vec4 baseColor{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    std::optional<TextureHandle> texture;
    SamplerState sampler;

    AlphaMode alphaMode =
        AlphaMode::Opaque;

    bool doubleSided = false;
};

struct RenderMeshSection {
    std::uint32_t firstIndex = 0;
    std::uint32_t indexCount = 0;
    std::uint32_t materialIndex = 0;

    // Generic depth offset in view-space depth units.
    // Negative values move the section slightly toward the camera.
    float depthBias = 0.0f;
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
