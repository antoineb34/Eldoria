#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>
#include <vector>

#include "ViewportViewDrawer.h"
#include "MapGpuViewportRenderer.h"

namespace eld::audio {
class MidiPlayer;
}

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
        eld::audio::MidiPlayer& midiPlayer,
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

    int midiVisualizationId_ = -1;
    int midiVisualizationTotalTicks_ = 0;
    std::vector<float> midiActivity_;

    int animationVisualizationId_ = -1;
};

}
