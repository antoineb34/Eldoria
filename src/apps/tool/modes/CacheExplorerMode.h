#pragma once

#include "ToolMode.h"

#include <imgui.h>

#include "../../../core/cache/CacheStore.h"
#include "../../../core/cache/ConfigArchiveLoader.h"

#include "cache/CacheTreeNode.h"

namespace rf::tool {

class CacheExplorerMode : public ToolMode {
public:
    CacheExplorerMode();

    bool initialize() override;

    void handleEvent(
        const SDL_Event& event
    ) override;

    void update() override;

    void renderUi() override;

    void render(
        SDL_Renderer* renderer,
        rf::render::DepthBuffer& depthBuffer,
        int windowWidth,
        int windowHeight
    ) override;

private:
    void inspectIndex0();

    rf::cache::CacheStore configCache_;
    rf::cache::ConfigArchiveLoader configLoader_;

    CacheTreeNode rootNode_;

    void buildRawCacheTree();
    void printTree(
        const CacheTreeNode& node,
        int depth
    );
};

}
