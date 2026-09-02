#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>

#include "viewport/ViewportControlsPanel.h"
#include "viewport/ViewportSurface.h"
#include "views/map/MapViewSurface.h"
#include "views/map/MapViewPanel.h"
#include "views/midi/MidiView.h"
#include "views/midi/MidiViewPanel.h"

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

class ViewportPanel {
public:
    void shutdown();

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
    // Renderer-backed scene image composited by ImGui.
    ViewportSurface viewportSurface_;

    // SDL renderer is stable for the lifetime of ElForge.
    // Stored after the first render pass so UI layout can resize
    // the target before ImGui records its texture reference.
    SDL_Renderer* viewportRenderer_ = nullptr;

    ViewportControlsPanel controlsPanel_;
    MapViewSurface mapGpuRenderer_;
    MapViewPanel mapViewPanel_;

    MidiView midiView_;
    MidiViewPanel midiViewPanel_;

};

}
