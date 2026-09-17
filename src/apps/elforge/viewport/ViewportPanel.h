#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>

#include "viewport/ViewportSurface.h"
#include "viewport/ViewportWorkspaceRouter.h"
#include "views/map/MapViewPanel.h"
#include "views/map/MapViewSurface.h"
#include "views/midi/MidiView.h"
#include "views/midi/MidiViewPanel.h"

namespace eld::audio {
class MidiPlayer;
}

namespace eld::graphics {
class ModelSystem;
}

namespace eld::render {
class ModelManager;
class TextureManager;
}

namespace eld::interface {
class WidgetLoader;
}

namespace eld::sprite {
class SpriteLoader;
}

namespace eld::elforge {

struct CacheExplorerState;

class ViewportPanel {
public:
  void shutdown();

  void render(CacheExplorerState &state, float width, float height,
              eld::audio::MidiPlayer &midiPlayer,
              const std::function<void()> &renderAnimationControls);

  void prepareViewport(
      SDL_Renderer *renderer,
      CacheExplorerState &state,
      eld::render::ModelManager &models,
      eld::render::TextureManager &textures);

  void renderViewport(
      SDL_Renderer *renderer,
      CacheExplorerState &state,
      eld::graphics::ModelSystem &modelSystem,
      eld::render::ModelManager &models,
      eld::render::TextureManager &textures,
      const eld::interface::WidgetLoader &interfaces,
      eld::sprite::SpriteLoader &interfaceSprites);

private:
  ViewportSurface viewportSurface_;

  SDL_Renderer *viewportRenderer_ = nullptr;

  ViewportWorkspaceRouter workspaceRouter_;
  MapViewSurface mapViewSurface_;
  MapViewPanel mapViewPanel_;

  MidiView midiView_;
  MidiViewPanel midiViewPanel_;
};

} // namespace eld::elforge
