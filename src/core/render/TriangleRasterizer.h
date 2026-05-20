#pragma once

#include <SDL3/SDL.h>

#include "Projection.h"

namespace rf::render {

void fillTriangle(
    SDL_Renderer* renderer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
);

}
