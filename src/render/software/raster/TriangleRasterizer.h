#pragma once

#include <SDL3/SDL.h>

#include "../../../core/assets/texture/TextureAsset.h"

#include "../camera/Projection.h"

namespace rf::render {

struct TextureMappingPoint {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

void fillTriangle(
    SDL_Renderer* renderer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
);

void fillTexturedTriangle(
    SDL_Renderer* renderer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c,

    const TextureMappingPoint& faceA,
    const TextureMappingPoint& faceB,
    const TextureMappingPoint& faceC,

    const TextureMappingPoint& textureOrigin,
    const TextureMappingPoint& textureU,
    const TextureMappingPoint& textureV,

    const rf::texture::TextureAsset& texture
);

}
