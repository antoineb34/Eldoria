#include "ToolApplication.h"

#include <array>
#include <iostream>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io =
        ImGui::GetIO();

    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(
        window,
        renderer
    );

    ImGui_ImplSDLRenderer3_Init(
        renderer
    );

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

            ImGui_ImplSDL3_ProcessEvent(
                &event
            );

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

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

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

        ImGui::Begin("RuneForge");

        if (ImGui::Button("Model Viewer")) {
            activeModeIndex = 0;
            activeMode = modes[activeModeIndex];
        }

        ImGui::SameLine();

        if (ImGui::Button("Cache Explorer")) {
            activeModeIndex = 1;
            activeMode = modes[activeModeIndex];
        }

        ImGui::Text(
            "Active mode: %d",
            activeModeIndex
        );

        ImGui::End();

        activeMode->renderUi();

        ImGui::Render();

        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            renderer
        );

        SDL_RenderPresent(
            renderer
        );

        SDL_Delay(16);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

}
