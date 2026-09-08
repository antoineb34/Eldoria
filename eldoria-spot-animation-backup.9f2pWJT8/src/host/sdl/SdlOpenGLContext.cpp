#include "SdlOpenGLContext.h"

#include <iostream>

namespace eld::host {

SdlOpenGLContext::SdlOpenGLContext(
    const char* title,
    int width,
    int height,
    int majorVersion,
    int minorVersion
) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << "\n";
        return;
    }

    if (
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_MAJOR_VERSION,
            majorVersion
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_MINOR_VERSION,
            minorVersion
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_DEPTH_SIZE,
            24
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_DOUBLEBUFFER,
            1
        )
    ) {
        std::cerr
            << "SDL_GL_SetAttribute failed: "
            << SDL_GetError()
            << "\n";
        SDL_Quit();
        return;
    }

    window_ = SDL_CreateWindow(
        title,
        width,
        height,
        SDL_WINDOW_OPENGL |
            SDL_WINDOW_RESIZABLE
    );

    if (window_ == nullptr) {
        std::cerr
            << "SDL_CreateWindow(OpenGL) failed: "
            << SDL_GetError()
            << "\n";
        SDL_Quit();
        return;
    }

    context_ = SDL_GL_CreateContext(window_);

    if (context_ == nullptr) {
        std::cerr
            << "SDL_GL_CreateContext failed: "
            << SDL_GetError()
            << "\n";
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return;
    }

    if (!SDL_GL_MakeCurrent(window_, context_)) {
        std::cerr
            << "SDL_GL_MakeCurrent failed: "
            << SDL_GetError()
            << "\n";
        SDL_GL_DestroyContext(context_);
        context_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return;
    }
}

SdlOpenGLContext::~SdlOpenGLContext() {
    if (context_ != nullptr) {
        SDL_GL_DestroyContext(context_);
        context_ = nullptr;
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

SDL_Window* SdlOpenGLContext::window() const {
    return window_;
}

SDL_GLContext SdlOpenGLContext::context() const {
    return context_;
}

bool SdlOpenGLContext::valid() const {
    return window_ != nullptr && context_ != nullptr;
}

bool SdlOpenGLContext::setSwapInterval(
    int interval
) const {
    if (!valid()) {
        return false;
    }

    return SDL_GL_SetSwapInterval(interval);
}

}
