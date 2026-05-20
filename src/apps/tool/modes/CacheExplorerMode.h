#pragma once

#include "ToolMode.h"

#include "../../../core/cache/CacheStore.h"
#include "../../../core/cache/ConfigArchiveLoader.h"

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

private:
    void inspectIndex0();

private:
    rf::cache::CacheStore configCache_;
    rf::cache::ConfigArchiveLoader configLoader_;
};

}
