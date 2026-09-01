#include "AppShell.h"
#include "../../platform/imgui/ImGuiTheme.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "../platform/sdl/SdlContext.h"

namespace eld::elforge {

int AppShell::run() {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    eld::platform::SdlContext sdl(
        "RuneForge Cache Explorer",
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (!window || !renderer) {
        return 1;
    }

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/jetbrains-mono-fonts/JetBrainsMono-Regular.otf",
        18.0f
    );

    ImGui::StyleColorsDark();
    eld::platform::imgui::applyImGuiTheme();

    ImGui_ImplSDL3_InitForSDLRenderer(
        window,
        renderer
    );

    ImGui_ImplSDLRenderer3_Init(
        renderer
    );

    if (!explorer_.initialize()) {
        return 1;
    }

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(
                &event
            );

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_EVENT_KEY_DOWN &&
                event.key.key == SDLK_ESCAPE
            ) {
                running = false;
            }

            explorer_.handleEvent(
                event
            );
        }

        // Map previews use the real OpenGL backend on a hidden tool surface.
        // Prepare before the SDL renderer starts this frame so the two GL
        // contexts never fight over pending renderer commands.
        explorer_.prepareViewport(
            renderer
        );

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        SDL_SetRenderDrawColor(
            renderer,
            18,
            20,
            22,
            255
        );

        SDL_RenderClear(
            renderer
        );

        explorer_.update();
        explorer_.renderUi();

        ImGui::Render();

        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            renderer
        );

        explorer_.renderViewport(
            renderer
        );

        SDL_RenderPresent(
            renderer
        );

        SDL_Delay(16);
    }

    explorer_.shutdown();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

}
