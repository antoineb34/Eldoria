#pragma once

#include <SDL3/SDL.h>

namespace rf::tool {

class Workspace {
public:
    virtual ~Workspace() = default;

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
        int viewportX,
        int viewportY,
        int viewportWidth,
        int viewportHeight
    ) = 0;
};

}
