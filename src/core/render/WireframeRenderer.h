#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include <SDL3/SDL.h>

#include "../model/FaceDecoder.h"
#include "../model/VertexDecoder.h"
#include "../texture/Texture.h"
#include "Projection.h"
#include "DepthBuffer.h"

namespace rf::render {

void drawWireframeModel(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const std::vector<rf::model::Vertex>& vertices,
    const std::vector<rf::model::Face>& faces,
    const std::unordered_map<int, rf::texture::DecodedTexture>& textures,
    const Camera& camera,
    bool showWireframe,
    bool showVertices,
    bool fillTriangles,
    bool useAlpha,
    bool highlightTexturedFaces
);

}
