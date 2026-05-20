#pragma once

#include <SDL3/SDL.h>

#include "Projection.h"
#include "DepthBuffer.h"

namespace rf::render {

void fillTriangle(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
);

}
