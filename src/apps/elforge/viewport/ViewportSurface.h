#pragma once

#include <SDL3/SDL.h>

namespace eld::elforge {

class ViewportSurface {
public:
    ViewportSurface() = default;
    ~ViewportSurface();

    ViewportSurface(
        const ViewportSurface&
    ) = delete;

    ViewportSurface& operator=(
        const ViewportSurface&
    ) = delete;

    bool ensure(
        SDL_Renderer* renderer,
        int width,
        int height
    );

    bool begin(
        SDL_Renderer* renderer
    );

    void end(
        SDL_Renderer* renderer
    );

    void shutdown();

    SDL_Texture* texture() const;

    int width() const;
    int height() const;

private:
    SDL_Texture* texture_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    SDL_Texture* previousTarget_ = nullptr;

    int width_ = 0;
    int height_ = 0;
};

}
