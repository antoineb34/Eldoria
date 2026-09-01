#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "graphics/GraphicsResources.h"
#include "render/RenderPipeline.h"
#include "render/backend/opengl/OpenGLRenderBackend.h"

#include "views/map/MapViewState.h"

namespace eld::elforge {

struct CacheExplorerState;

class MapViewSurface {
public:
    MapViewSurface() = default;
    ~MapViewSurface();

    MapViewSurface(
        const MapViewSurface&
    ) = delete;

    MapViewSurface& operator=(
        const MapViewSurface&
    ) = delete;

    void shutdown();

    bool prepare(
        SDL_Renderer* renderer,
        CacheExplorerState& state,
        eld::graphics::GraphicsResources& resources
    );

    void draw(
        SDL_Renderer* renderer,
        const CacheExplorerState& state
    ) const;

    const std::string& error() const;

private:
    bool ensureOpenGLContext(
        int width,
        int height
    );

    bool ensureOutputTexture(
        SDL_Renderer* renderer,
        int width,
        int height
    );

    bool makeMapContextCurrent();
    void destroyOpenGLContext();
    void destroyOutputTexture();

    void flipReadbackRows(
        int width,
        int height
    );

    SDL_Window* gpuWindow_ = nullptr;
    SDL_GLContext gpuContext_ = nullptr;

    std::unique_ptr<eld::render::OpenGLRenderBackend>
        backend_;

    eld::render::RenderPipeline pipeline_;

    SDL_Texture* outputTexture_ = nullptr;
    SDL_Renderer* outputRenderer_ = nullptr;

    int width_ = 0;
    int height_ = 0;

    std::vector<std::uint8_t> pixels_;
    std::vector<std::uint8_t> flippedPixels_;

    std::string error_;
};

}
