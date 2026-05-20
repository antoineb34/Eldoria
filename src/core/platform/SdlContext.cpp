#include "SdlContext.h"

#include <iostream>

namespace rf::platform {

SdlContext::SdlContext(
    const char* title,
    int width,
    int height
) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {

        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << "\n";

        return;
    }

    window_ =
        SDL_CreateWindow(
            title,
            width,
            height,
            SDL_WINDOW_RESIZABLE
        );

    if (!window_) {

        std::cerr
            << "SDL_CreateWindow failed: "
            << SDL_GetError()
            << "\n";

        SDL_Quit();

        return;
    }

    renderer_ =
        SDL_CreateRenderer(
            window_,
            nullptr
        );

    if (!renderer_) {

        std::cerr
            << "SDL_CreateRenderer failed: "
            << SDL_GetError()
            << "\n";

        SDL_DestroyWindow(window_);

        window_ = nullptr;

        SDL_Quit();

        return;
    }
}

SdlContext::~SdlContext() {

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    SDL_Quit();
}

SDL_Window* SdlContext::window() const {

    return window_;
}

SDL_Renderer* SdlContext::renderer() const {

    return renderer_;
}

}
