#include "ToolApplication.h"

#include <array>
#include <iostream>

#include <SDL3/SDL.h>

#include "../../core/platform/SdlContext.h"
#include "../../core/render/DepthBuffer.h"

#include "modes/ModelViewerMode.h"
#include "modes/CacheExplorerMode.h"

namespace rf::tool {

int ToolApplication::run() {

    constexpr int WINDOW_WIDTH = 960;
    constexpr int WINDOW_HEIGHT = 640;

    rf::platform::SdlContext sdl(
        "RuneForge Tool",
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    SDL_Window* window =
        sdl.window();

    SDL_Renderer* renderer =
        sdl.renderer();

    if (!window || !renderer) {
        return 1;
    }

    ModelViewerMode modelViewer;
    CacheExplorerMode cacheExplorer;

    if (!modelViewer.initialize()) {
        return 1;
    }

    if (!cacheExplorer.initialize()) {
        return 1;
    }

    std::array<ToolMode*, 2> modes {
        &modelViewer,
        &cacheExplorer
    };

    int activeModeIndex = 0;

    ToolMode* activeMode =
        modes[activeModeIndex];

    rf::render::DepthBuffer depthBuffer(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    bool running = true;

    while (running) {

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (
                event.type ==
                SDL_EVENT_QUIT
            ) {
                running = false;
            }

            if (
                event.type ==
                SDL_EVENT_KEY_DOWN &&
                event.key.key ==
                SDLK_ESCAPE
            ) {
                running = false;
            }

            if (
                event.type ==
                SDL_EVENT_KEY_DOWN &&
                event.key.key ==
                SDLK_TAB
            ) {
                activeModeIndex =
                    (activeModeIndex + 1) %
                    modes.size();

                activeMode =
                    modes[activeModeIndex];

                std::cout
                    << "\nSwitched mode: "
                    << activeModeIndex
                    << "\n";
            }

            activeMode->handleEvent(
                event
            );
        }

        int windowWidth = 0;
        int windowHeight = 0;

        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        depthBuffer.resize(
            windowWidth,
            windowHeight
        );

        depthBuffer.clear();

        activeMode->update();

        activeMode->render(
            renderer,
            depthBuffer,
            windowWidth,
            windowHeight
        );

        SDL_RenderPresent(
            renderer
        );

        SDL_Delay(16);
    }

    return 0;
}

}
