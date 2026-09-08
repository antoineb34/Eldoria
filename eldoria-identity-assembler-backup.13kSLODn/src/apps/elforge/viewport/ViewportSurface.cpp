#include "viewport/ViewportSurface.h"

#include <algorithm>

namespace eld::elforge {

ViewportSurface::~ViewportSurface() {
    shutdown();
}


bool ViewportSurface::ensure(
    SDL_Renderer* renderer,
    int width,
    int height
) {
    if (renderer == nullptr) {
        return false;
    }

    width =
        std::max(
            width,
            1
        );

    height =
        std::max(
            height,
            1
        );

    if (
        texture_ != nullptr &&
        renderer_ == renderer &&
        width_ == width &&
        height_ == height
    ) {
        return true;
    }

    shutdown();

    texture_ =
        SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            width,
            height
        );

    if (texture_ == nullptr) {
        return false;
    }

    renderer_ =
        renderer;

    width_ =
        width;

    height_ =
        height;

    SDL_SetTextureScaleMode(
        texture_,
        SDL_SCALEMODE_LINEAR
    );

    // The viewport is an opaque scene surface.
    // ImGui owns final application composition.
    SDL_SetTextureBlendMode(
        texture_,
        SDL_BLENDMODE_NONE
    );

    return true;
}


bool ViewportSurface::begin(
    SDL_Renderer* renderer
) {
    if (
        texture_ == nullptr ||
        renderer == nullptr ||
        renderer != renderer_
    ) {
        return false;
    }

    previousTarget_ =
        SDL_GetRenderTarget(
            renderer
        );

    if (
        !SDL_SetRenderTarget(
            renderer,
            texture_
        )
    ) {
        previousTarget_ =
            nullptr;

        return false;
    }

    SDL_SetRenderViewport(
        renderer,
        nullptr
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );

    return true;
}


void ViewportSurface::end(
    SDL_Renderer* renderer
) {
    if (renderer == nullptr) {
        previousTarget_ =
            nullptr;

        return;
    }

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );

    SDL_SetRenderViewport(
        renderer,
        nullptr
    );

    SDL_SetRenderTarget(
        renderer,
        previousTarget_
    );

    previousTarget_ =
        nullptr;

    // Target dimensions changed when restoring the window.
    // Reset these again for the restored target.
    SDL_SetRenderViewport(
        renderer,
        nullptr
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}


void ViewportSurface::shutdown() {
    if (texture_ != nullptr) {
        SDL_DestroyTexture(
            texture_
        );

        texture_ =
            nullptr;
    }

    renderer_ =
        nullptr;

    previousTarget_ =
        nullptr;

    width_ =
        0;

    height_ =
        0;
}


SDL_Texture* ViewportSurface::texture() const {
    return texture_;
}


int ViewportSurface::width() const {
    return width_;
}


int ViewportSurface::height() const {
    return height_;
}

}
