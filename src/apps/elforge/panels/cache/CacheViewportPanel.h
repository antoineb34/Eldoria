#pragma once

#include <SDL3/SDL.h>
#include "CacheState.h"

namespace eldoria::apps::elforge {

class CacheViewportPanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );

    void renderViewport(
        SDL_Renderer* renderer,
        const CacheState& state
    );
};

}
