#pragma once

#include "ToolMode.h"
#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

#include "../../../core/cache/ArchiveFileTable.h"
#include "../../../core/cache/CacheStore.h"
#include "../../../core/cache/ConfigArchiveLoader.h"

#include "../../core/texture/Texture.h"

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
        int viewportX,
        int viewportY,
        int viewportWidth,
        int viewportHeight
    ) override;

    void renderUi() override;

    void uploadPreviewTexture(
        SDL_Renderer* renderer
    );

private:

    rf::texture::DecodedTexture previewTextureData_;

    SDL_Texture* previewTexture_ = nullptr;

    void inspectSelectedArchive();
    void inspectSelectedFile();
    std::vector<std::uint8_t> toBytes(const std::vector<char>& data) const;

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
