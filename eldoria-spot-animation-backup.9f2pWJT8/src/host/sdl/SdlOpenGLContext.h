#pragma once

#include <SDL3/SDL.h>

namespace eld::host {

class SdlOpenGLContext {
public:
    SdlOpenGLContext(
        const char* title,
        int width,
        int height,
        int majorVersion = 3,
        int minorVersion = 3
    );

    ~SdlOpenGLContext();

    SdlOpenGLContext(const SdlOpenGLContext&) = delete;
    SdlOpenGLContext& operator=(const SdlOpenGLContext&) = delete;

    SDL_Window* window() const;
    SDL_GLContext context() const;

    bool valid() const;
    bool setSwapInterval(int interval) const;

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
};

}
