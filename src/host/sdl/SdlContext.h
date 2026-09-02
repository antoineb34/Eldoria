#pragma once

#include <SDL3/SDL.h>

namespace eld::host {

class SdlContext {
public:

    SdlContext(
        const char* title,
        int width,
        int height
    );

    ~SdlContext();

    SDL_Window* window() const;

    SDL_Renderer* renderer() const;

private:

    SDL_Window* window_ = nullptr;

    SDL_Renderer* renderer_ = nullptr;
};

}
