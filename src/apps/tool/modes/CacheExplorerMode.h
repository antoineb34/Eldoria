#pragma once

#include "ToolMode.h"

#include <cstdint>

#include "../../../core/cache/ArchiveFileTable.h"
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

    void render(
        SDL_Renderer* renderer,
        rf::render::DepthBuffer& depthBuffer,
        int windowWidth,
        int windowHeight
    ) override;

    void renderUi() override;

private:
    void buildRawCacheTree();

    CacheTreeNode makeFileNode(
        uint32_t archiveId,
        int fileIndex,
        const rf::cache::ArchiveFileEntry& file
    ) const;

    void renderTreeNode(
        const CacheTreeNode& node
    );

    void renderInspector();

private:
    rf::cache::CacheStore configCache_;
    rf::cache::ConfigArchiveLoader configLoader_;

    CacheTreeNode rootNode_;

    CacheTreeNode selectedNode_;
    bool hasSelection_ = false;
};

}
