#include "CacheExplorer.h"
#include <imgui.h>

namespace rf::explorer {

    CacheExplorer::CacheExplorer()
        : textureLoader_(cache_),
          modelLoader_(cache_, textureLoader_) {
    }

    bool CacheExplorer::initialize() {

        state_.camera.angleX = 0.45f;
        state_.camera.angleY = 0.6f;
        state_.camera.distance = 1200.0f;

        state_.camera.fov = 0.35f;
        state_.camera.nearPlane = 1.0f;
        state_.camera.farPlane = 10000.0f;

        state_.camera.viewportX = 0;
        state_.camera.viewportY = 0;
        state_.camera.viewportWidth = 1;
        state_.camera.viewportHeight = 1;state_.camera.angleX = 0.0f;
        state_.camera.angleY = 0.0f;
        state_.camera.distance = 500.0f;

        state_.camera.fov = 1.04719755f;
        state_.camera.nearPlane = 1.0f;
        state_.camera.farPlane = 10000.0f;

        state_.camera.viewportX = 0;
        state_.camera.viewportY = 0;
        state_.camera.viewportWidth = 1;
        state_.camera.viewportHeight = 1;

        state_.modelTransform.scale = 1.0f;
        state_.modelTransform.rotationX = 0.0f;
        state_.modelTransform.rotationY = 0.0f;
        state_.modelTransform.rotationZ = 0.0f;
        state_.modelTransform.offsetX = 0.0f;
        state_.modelTransform.offsetY = 0.0f;
        state_.modelTransform.offsetZ = 0.0f;

        state_.renderOptions.fillTriangles = true;
        state_.renderOptions.showWireframe = false;
        state_.renderOptions.showVertices = false;
        state_.renderOptions.highlightTexturedFaces = false;
        state_.renderOptions.useAlpha = true;

        if (!cache_.isValid()) {
            return false;
        }

        state_.rootNode =
            treeBuilder_.build(cache_);

        return true;
    }

void CacheExplorer::handleEvent(
    const SDL_Event& event
) {
    (void) event;
}

void CacheExplorer::update() {
    if (state_.selection.label != lastSelectedLabel_) {
        lastSelectedLabel_ =
            state_.selection.label;

        handleSelectionChanged();
    }
}

void CacheExplorer::handleSelectionChanged() {
    state_.activeModel.reset();
    state_.activeTexture.reset();

    switch (state_.selection.type) {
        case CacheTreeNodeType::Root:
            break;

        case CacheTreeNodeType::Index:
            break;

        case CacheTreeNodeType::File:
            break;

        case CacheTreeNodeType::Model: {
            if (state_.selection.fileId < 0) {
                break;
            }

            state_.activeModel =
                modelLoader_.load(
                    static_cast<std::uint32_t>(
                        state_.selection.fileId
                    )
                );

            break;
        }

        case CacheTreeNodeType::Texture:
            break;
    }
}

void CacheExplorer::renderViewport(
    SDL_Renderer* renderer
) {
    viewportPanel_.renderViewport(
        renderer,
        state_
    );
}

void CacheExplorer::renderUi() {
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

    ImGui::Begin(
        "CacheExplorer",
        nullptr,
        shellFlags
    );

    ImGui::TextUnformatted(
        "RuneForge Cache Explorer"
    );

    ImGui::Separator();

    const float treeWidth = 300.0f;
    const float inspectorWidth = 320.0f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const float height =
        available.y;

    float viewportWidth =
        available.x - treeWidth - inspectorWidth - spacing * 2.0f;

    if (viewportWidth < 100.0f) {
        viewportWidth = 100.0f;
    }

    treePanel_.render(
        state_,
        treeWidth,
        height
    );

    ImGui::SameLine();

    viewportPanel_.render(
        state_,
        viewportWidth,
        height
    );

    ImGui::SameLine();

    inspectorPanel_.render(
        state_,
        inspectorWidth,
        height
    );

    ImGui::End();
}

}
