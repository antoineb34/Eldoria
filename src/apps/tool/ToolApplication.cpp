#include "ToolApplication.h"

#include <array>
#include <cstdlib>
#include <iostream>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "../../core/platform/SdlContext.h"
#include "../../core/render/DepthBuffer.h"

#include "modes/CacheExplorerMode.h"
#include "modes/ModelViewerMode.h"
#include "../../ui/ImGuiTheme.h"

namespace rf::tool {

void clearTerminal() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

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

    activeMode->onEnter();

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

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_EVENT_KEY_DOWN &&
                event.key.key == SDLK_ESCAPE
            ) {
                running = false;
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
            "RuneForgeShell",
            nullptr,
            shellFlags
        );

        if (ImGui::BeginTabBar("RuneForgeTabs")) {
            if (ImGui::BeginTabItem("Model Viewer")) {
                if (activeModeIndex != 0) {
                    activeModeIndex = 0;
                    activeMode = modes[activeModeIndex];
                    clearTerminal();
                    activeMode->onEnter();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Cache Explorer")) {
                if (activeModeIndex != 1) {
                    activeModeIndex = 1;
                    activeMode = modes[activeModeIndex];
                    clearTerminal();
                    activeMode->onEnter();
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Separator();

        ImGui::BeginChild(
            "ControlsPanel",
            ImVec2(300.0f, 0.0f),
            true
        );

        activeMode->renderUi();

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild(
            "ViewportPanel",
            ImVec2(0.0f, 0.0f),
            true
        );

        ImVec2 viewportPos =
            ImGui::GetWindowPos();

        ImVec2 viewportSize =
            ImGui::GetWindowSize();

        viewportX =
            static_cast<int>(
                viewportPos.x
            );

        viewportY =
            static_cast<int>(
                viewportPos.y
            );

        viewportWidth =
            static_cast<int>(
                viewportSize.x
            );

        viewportHeight =
            static_cast<int>(
                viewportSize.y
            );

        ImGui::EndChild();

        ImGui::End();

        activeMode->update();

        activeMode->render(
            renderer,
            depthBuffer,
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
