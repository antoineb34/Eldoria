#pragma once

#include <SDL3/SDL.h>

#include "software/DepthBuffer.h"

namespace rf::tool {

class ToolMode {
public:
    virtual ~ToolMode() = default;

    virtual bool initialize() = 0;

    virtual void onEnter() {
    }

    virtual void handleEvent(
        const SDL_Event& event
    ) = 0;

    virtual void update() = 0;

    virtual void renderUi() = 0;

    virtual void render(
        SDL_Renderer* renderer,
        rf::render::DepthBuffer& depthBuffer,
        int viewportX,
        int viewportY,
        int viewportWidth,
        int viewportHeight
    ) = 0;
};

}
