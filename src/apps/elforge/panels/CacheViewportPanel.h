#pragma once

#include <SDL3/SDL.h>

#include "ViewportViewDrawer.h"

namespace eld::graphics {
class GraphicsResources;
}

namespace eld::interface {
class InterfaceRepository;
}

namespace eld::sprite {
class SpriteRepository;
}

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
        CacheExplorerState& state,
        eld::graphics::GraphicsResources& resources,
        const eld::interface::InterfaceRepository& interfaces,
        eld::sprite::SpriteRepository& interfaceSprites
    );

private:
    ViewportViewDrawer viewDrawer_;
};

}
