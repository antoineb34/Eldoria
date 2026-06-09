#pragma once

#include <SDL3/SDL.h>

#include "cache/Cache.h"

#include "CacheExplorerState.h"
#include "CacheTreeBuilder.h"

#include "panels/CacheTreePanel.h"
#include "panels/CacheInspectorPanel.h"
#include "panels/CacheViewportPanel.h"

#include "../../core/assets/texture/TextureLoader.h"
#include "../../core/assets/model/ModelLoader.h"

namespace rf::explorer {

class CacheExplorer {
public:
    CacheExplorer();

    bool initialize();

    void handleEvent(const SDL_Event& event);
    void update();
    void renderUi();
    void renderViewport(
        SDL_Renderer* renderer
    );

private:
    rf::cache::Cache cache_;
    rf::texture::TextureLoader textureLoader_;
    rf::model::ModelLoader modelLoader_;

    CacheExplorerState state_;
    CacheTreeBuilder treeBuilder_;

    CacheTreePanel treePanel_;
    CacheViewportPanel viewportPanel_;
    CacheInspectorPanel inspectorPanel_;

private:
    void handleSelectionChanged();

    std::string lastSelectedLabel_;
    void findNextAlphaModel();
};

}
