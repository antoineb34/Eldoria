#include "StudioApplication.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "../platform/sdl/SdlContext.h"
#include "../ui/imgui/ImGuiTheme.h"

#include "workspace/Workspace.h"
#include "workspace/cache/CacheWorkspace.h"

namespace rf::tool {

int StudioApplication::run() {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    rf::platform::SdlContext sdl(
        "RuneForge Studio",
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

    io.Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/jetbrains-mono-fonts/JetBrainsMono-Regular.otf",
        18.0f
    );

    ImGui::StyleColorsDark();

    rf::ui::applyImGuiTheme();

    ImGui_ImplSDL3_InitForSDLRenderer(
        window,
        renderer
    );

    ImGui_ImplSDLRenderer3_Init(
        renderer
    );

    CacheWorkspace cacheWorkspace;

    Workspace* activeWorkspace =
        &cacheWorkspace;

    if (!activeWorkspace->initialize()) {
        return 1;
    }

    activeWorkspace->onEnter();

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

            activeWorkspace->handleEvent(
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

        ImGuiViewport* viewport =
            ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(
            viewport->WorkPos
        );

        ImGui::SetNextWindowSize(
            viewport->WorkSize
        );

        ImGuiWindowFlags shellFlags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        int viewportX = 0;
        int viewportY = 0;
        int viewportWidth = 1;
        int viewportHeight = 1;

        ImGui::Begin(
            "RuneForgeStudio",
            nullptr,
            shellFlags
        );

        activeWorkspace->renderUi();

        ImVec2 viewportPos =
            ImGui::GetCursorScreenPos();

        ImVec2 viewportSize =
            ImGui::GetContentRegionAvail();

        viewportX = static_cast<int>(viewportPos.x);
        viewportY = static_cast<int>(viewportPos.y);
        viewportWidth = static_cast<int>(viewportSize.x);
        viewportHeight = static_cast<int>(viewportSize.y);

        ImGui::End();

        activeWorkspace->update();

        activeWorkspace->render(
            renderer,
            viewportX,
            viewportY,
            viewportWidth,
            viewportHeight
        );

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
