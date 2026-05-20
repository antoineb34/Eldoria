#pragma once

#include <SDL3/SDL.h>

#include "../../../core/render/DepthBuffer.h"

namespace rf::tool {

class ToolMode {
public:
    virtual ~ToolMode() = default;

    virtual bool initialize() = 0;

    virtual void handleEvent(
        const SDL_Event& event
    ) = 0;

    virtual void update() = 0;

    virtual void render(
        SDL_Renderer* renderer,
        rf::render::DepthBuffer& depthBuffer,
        int windowWidth,
        int windowHeight
    ) = 0;
};

}
