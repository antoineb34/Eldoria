#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <SDL3/SDL.h>
#include "panels/cache/CacheState.h"
#include "panels/cache/CacheTreeBuilder.h"
#include "panels/cache/CacheTreePanel.h"
#include "panels/cache/CacheInspectorPanel.h"
#include "panels/cache/CacheViewportPanel.h"

#include "../../platform/sdl/SdlContext.h"
#include "cache/Cache.h"
#include "model/ModelLoader.h"
#include "../../core/assets/texture/TextureLoader.h"

namespace eldoria::apps::elforge {

class ElForgeApplication {
public:
    ElForgeApplication();
    ~ElForgeApplication();

    int run();

private:
    bool initialize();
    void shutdown();
    void handleEvent(const SDL_Event& event);
    void update();
    void render();

    void handleSelectionChanged();
    std::optional<rf::texture::TextureAsset> loadModelTexture(
        std::uint32_t id
    );

private:
    // Platform
    std::unique_ptr<rf::platform::SdlContext> sdl_;

    // Core systems
    rf::cache::Cache cache_;
    rf::texture::TextureLoader textureLoader_;
    rf::model::ModelLoader modelLoader_;

    // Tool state
    CacheState state_;
    CacheTreeBuilder treeBuilder_;

    // Panels
    CacheTreePanel treePanel_;
    CacheInspectorPanel inspectorPanel_;
    CacheViewportPanel viewportPanel_;

    // Shell state
    std::string lastSelectedLabel_;
    bool running_ = false;
};

}
