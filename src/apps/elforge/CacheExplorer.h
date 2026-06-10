#pragma once

#include <SDL3/SDL.h>

#include "cache/Cache.h"

#include "CacheExplorerState.h"
#include "CacheTreeBuilder.h"

#include "panels/CacheTreePanel.h"
#include "panels/CacheInspectorPanel.h"
#include "panels/CacheViewportPanel.h"

#include "model/ModelLoader.h"
#include "texture/TextureLoader.h"

namespace eld::elforge {

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
    eld::cache::Cache cache_;
    eld::texture::TextureLoader textureLoader_;
    eld::model::ModelLoader modelLoader_;

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
