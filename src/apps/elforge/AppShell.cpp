#include "AppShell.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "ImGuiTheme.h"
#include "sdl/SdlContext.h"

namespace eld::elforge {

namespace {

constexpr char AppTitle[] = "ElForge";
constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;
constexpr float UiFontSize = 18.0f;

void configureFonts(ImGuiIO& io) {
    ImFont* font =
        io.Fonts->AddFontFromFileTTF(
            "/usr/share/fonts/jetbrains-mono-fonts/JetBrainsMono-Regular.otf",
            UiFontSize
        );

    if (font == nullptr) {
        io.Fonts->AddFontDefault();
    }
}

}

int AppShell::run() {
    eld::platform::SdlContext sdl(
        AppTitle,
        WindowWidth,
        WindowHeight
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

    configureFonts(io);

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
