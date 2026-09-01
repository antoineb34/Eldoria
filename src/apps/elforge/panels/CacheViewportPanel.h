#pragma once

#include <SDL3/SDL.h>

#include <functional>

#include "ViewportViewDrawer.h"
#include "MapGpuViewportRenderer.h"

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
    void shutdown();

    // ELFORGE_NPC_ANIMATION_DRAWER_V1
    void render(
        CacheExplorerState& state,
        float width,
        float height,
        const std::function<void()>&
            renderAnimationControls
    );

    void prepareViewport(
        SDL_Renderer* renderer,
        CacheExplorerState& state,
        eld::graphics::GraphicsResources& resources
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
    MapGpuViewportRenderer mapGpuRenderer_;
};

}
