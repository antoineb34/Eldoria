#include "ElForgeApplication.h"

#include <iostream>
#include <utility>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "cache/CacheFileDetails.h"

#include "../../platform/sdl/SdlContext.h"
#include "ui/ElForgeImGuiTheme.h"

namespace eldoria::apps::elforge {

ElForgeApplication::ElForgeApplication()
    : textureLoader_(cache_),
      modelLoader_(
          cache_,
          [this](std::uint32_t id) {
              return loadModelTexture(id);
          }
      ) {
}

ElForgeApplication::~ElForgeApplication() {
    shutdown();
}

int ElForgeApplication::run() {
    if (!initialize()) {
        return 1;
    }

    running_ = true;

    while (running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleEvent(event);
        }

        update();
        render();

        SDL_Delay(16);
    }

    return 0;
}

bool ElForgeApplication::initialize() {
    constexpr int WINDOW_WIDTH = 1600;
    constexpr int WINDOW_HEIGHT = 800;

    sdl_ = std::make_unique<rf::platform::SdlContext>(
        "ElForge",
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    if (!sdl_->window() || !sdl_->renderer()) {
        return false;
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
    applyElForgeImGuiTheme();

    ImGui_ImplSDL3_InitForSDLRenderer(sdl_->window(), sdl_->renderer());
    ImGui_ImplSDLRenderer3_Init(sdl_->renderer());

    state_.modelViewportCamera.angleX = 0.0f;
    state_.modelViewportCamera.angleY = 0.0f;
    state_.modelViewportCamera.distance = 500.0f;
    state_.modelViewportCamera.fov = 1.04719755f;
    state_.modelViewportCamera.nearPlane = 1.0f;
    state_.modelViewportCamera.farPlane = 10000.0f;

    state_.modelViewportTransform.scale = 1.0f;
    state_.modelViewportRenderOptions.fillTriangles = true;
    state_.modelViewportRenderOptions.useAlpha = true;

    if (cache_.isValid()) {
        state_.rootNode = treeBuilder_.build(cache_);
    }

    return true;
}

void ElForgeApplication::shutdown() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void ElForgeApplication::handleEvent(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
    }

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        running_ = false;
    }
}

void ElForgeApplication::update() {
    bool selectionChanged = false;
    if (state_.selection.label != lastSelectedLabel_) {
        lastSelectedLabel_ = state_.selection.label;
        selectionChanged = true;
    }

    // Handle pending texture load from browser input
    if (state_.pendingTextureLoad) {
        state_.pendingTextureLoad = false;
        std::uint32_t id = static_cast<std::uint32_t>(state_.textureInputId);
        textureInspector_.loadFromCache(textureLoader_, id);

        if (textureInspector_.result.loaded) {
            state_.selectedTexture = std::move(textureInspector_.result.asset);
            state_.selectedTextureLoadError = std::nullopt;
        } else {
            state_.selectedTexture = std::nullopt;
            state_.selectedTextureLoadError = textureInspector_.result.message;
        }
    }

    if (selectionChanged) {
        handleSelectionChanged();
    }
}

void ElForgeApplication::handleSelectionChanged() {
    state_.selectedModel.reset();
    state_.selectedModelLoadError.reset();
    state_.selectedCacheFileDetails.reset();
    state_.selectedModelTexture.reset();

    if (state_.selection.fileId >= 0 && state_.selection.indexId >= 0) {
        auto file = cache_.readFile(
            static_cast<rf::cache::CacheIndex>(state_.selection.indexId),
            state_.selection.fileId
        );

        if (file.has_value()) {
            state_.selectedCacheFileDetails =
                rf::cache::inspectCacheFile(*file);
        }
    }

    if (state_.selection.type == CacheTreeNodeType::Model) {
        if (state_.selection.fileId >= 0) {
            rf::model::ModelLoadResult result =
                modelLoader_.loadDetailed(
                    static_cast<std::uint32_t>(state_.selection.fileId)
                );

            if (result.loaded()) {
                state_.selectedModel =
                    std::move(*result.asset);
            } else {
                state_.selectedModelLoadError =
                    result.message;
            }
        }
    }
}

std::optional<rf::texture::TextureAsset> ElForgeApplication::loadModelTexture(
    std::uint32_t id
) {
    return textureLoader_.load(id);
}

void ElForgeApplication::render() {
    SDL_Renderer* renderer = sdl_->renderer();

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("ElForgeShell", nullptr, windowFlags);

    ImGui::TextUnformatted("ElForge - Eldoria Development Tool");
    ImGui::Separator();

    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float contentH = ImGui::GetContentRegionAvail().y;

    // 4-column layout: tree | viewport | inspector | texture_panels
    const float colTree = 220.0f;
    const float colInspector = 300.0f;
    const float colTex = 320.0f;
    float colViewport = ImGui::GetContentRegionAvail().x - colTree - colInspector - colTex - spacing * 3.0f;
    if (colViewport < 200.0f) colViewport = 200.0f;

    // --- Column 1: Cache tree ---
    ImGui::BeginChild("##col_tree", ImVec2(colTree, contentH), false);
    treePanel_.render(state_, colTree, contentH);
    ImGui::EndChild();

    ImGui::SameLine();

    // --- Column 2: Model viewport ---
    viewportPanel_.render(state_, colViewport, contentH);

    ImGui::SameLine();

    // --- Column 3: Inspector ---
    inspectorPanel_.render(state_, colInspector, contentH);

    ImGui::SameLine();

    // --- Column 4: Texture panels (vertical stack) ---
    ImGui::BeginChild("##col_texture", ImVec2(colTex, contentH), false);
    float texAvailH = ImGui::GetContentRegionAvail().y;
    const float browserH = 110.0f;
    const float metaH = 220.0f;
    float previewH = texAvailH - browserH - metaH - spacing * 2.0f;
    if (previewH < 80.0f) previewH = 80.0f;

    textureBrowserPanel_.render(state_, colTex, browserH);
    texturePreviewPanel_.render(state_, colTex, previewH);
    textureMetadataPanel_.render(state_, colTex, metaH);

    ImGui::EndChild();

    ImGui::End();

    // Render SDL output
    SDL_SetRenderDrawColor(renderer, 18, 20, 22, 255);
    SDL_RenderClear(renderer);

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    // Render model viewport overlay
    viewportPanel_.renderViewport(renderer, state_);

    SDL_RenderPresent(renderer);
}

}
