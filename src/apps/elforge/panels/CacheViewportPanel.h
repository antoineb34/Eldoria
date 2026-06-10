#pragma once

#include <SDL3/SDL.h>

namespace eld::elforge {

struct CacheExplorerState;

class CacheViewportPanel {
public:
    void render(
        CacheExplorerState& state,
        float width,
        float height
    );

    void renderViewport(
        SDL_Renderer* renderer,
        CacheExplorerState& state
    );
};

}
