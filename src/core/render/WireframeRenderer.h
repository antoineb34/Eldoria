#pragma once

#include <vector>

#include <SDL3/SDL.h>

#include "../model/FaceDecoder.h"
#include "../model/VertexDecoder.h"
#include "Projection.h"

namespace rf::render {

void drawWireframeModel(
    SDL_Renderer* renderer,
    const std::vector<rf::model::Vertex>& vertices,
    const std::vector<rf::model::Face>& faces,
    const Camera& camera
);

}
