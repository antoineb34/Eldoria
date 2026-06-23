#pragma once

#include <string>

#include <SDL3/SDL.h>

#include "cache/Cache.h"
#include "cache_legacy/Cache.h"

#include "CacheExplorerState.h"
#include "CacheTreeBuilder.h"

#include "panels/CacheInspectorPanel.h"
#include "panels/CacheTreePanel.h"
#include "panels/CacheViewportPanel.h"

#include "model/ModelRepository.h"
#include "texture/TextureRepository.h"

namespace eld::elforge {

class CacheExplorer {
public:
    CacheExplorer();

    bool initialize();

    void handleEvent(
        const SDL_Event& event
    );

    void update();
    void renderUi();

    void renderViewport(
        SDL_Renderer* renderer
    );

private:
    bool hasAlphaFaces(
        const eld::model::ModelMesh& model
    ) const;

    void handleSelectionChanged();
    void findNextAlphaModel();

    eld::cache::Cache cache_;
    eld::cache_legacy::Cache legacyCache_;

    eld::texture::TextureRepository textureRepository_;
    eld::model::ModelRepository modelRepository_;

    CacheExplorerState state_;
    CacheTreeBuilder treeBuilder_;

    CacheTreePanel treePanel_;
    CacheViewportPanel viewportPanel_;
    CacheInspectorPanel inspectorPanel_;

    std::string lastSelectedLabel_;
};

}
