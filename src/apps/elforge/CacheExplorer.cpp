#include "CacheExplorer.h"

#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include <imgui.h>

namespace eld::elforge {

bool CacheExplorer::hasAlphaFaces(
    const eld::model::ModelAsset& model
) const {
    for (const eld::model::Face& face : model.faces) {
        if (face.alpha > 0) {
            return true;
        }
    }

    return false;
}

void CacheExplorer::findNextAlphaModel() {
    int startId = 0;

    if (
        state_.activeModel &&
        state_.selection.fileId >= 0
    ) {
        startId =
            state_.selection.fileId + 1;
    }

    const std::vector<std::uint16_t> modelIds =
        modelRepository_.listIds();

    for (const std::uint16_t modelId : modelIds) {
        if (
            static_cast<int>(modelId) <
            startId
        ) {
            continue;
        }

        try {
            eld::model::Model model =
                modelRepository_.get(modelId);

            if (!hasAlphaFaces(model.asset)) {
                continue;
            }

            state_.activeModel =
                std::move(model);

            state_.activeTexture.reset();

            state_.selection.type =
                CacheTreeNodeType::Model;

            state_.selection.fileId =
                static_cast<int>(modelId);

            state_.selection.label =
                "Model " +
                std::to_string(modelId);

            lastSelectedLabel_ =
                state_.selection.label;

            return;
        }
        catch (const std::exception&) {
            continue;
        }
    }
}

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      legacyCache_("cache"),
      textureLoader_(legacyCache_),
      modelRepository_(
          cache_.open(
              eld::cache::IndexId::Models
          )
      ) {
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
    state_.camera.viewportHeight = 1;

    state_.camera.angleX = 0.0f;
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

    if (!legacyCache_.isValid()) {
        return false;
    }

    state_.rootNode =
        treeBuilder_.build(
            legacyCache_
        );

    return true;
}

void CacheExplorer::handleEvent(
    const SDL_Event& event
) {
    (void) event;
}

void CacheExplorer::update() {
    if (
        state_.selection.label !=
        lastSelectedLabel_
    ) {
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
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const std::uint16_t modelId =
                static_cast<std::uint16_t>(
                    state_.selection.fileId
                );

            try {
                std::optional<eld::model::Model> model =
                    modelRepository_.find(modelId);

                if (model.has_value()) {
                    state_.activeModel =
                        std::move(*model);
                }
            }
            catch (const std::exception&) {
                state_.activeModel.reset();
            }

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

    if (ImGui::Button("Find alpha model")) {
        findNextAlphaModel();
    }

    const float treeWidth = 300.0f;
    const float inspectorWidth = 320.0f;
    const float spacing =
        ImGui::GetStyle().ItemSpacing.x;

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const float height =
        available.y;

    float viewportWidth =
        available.x -
        treeWidth -
        inspectorWidth -
        spacing * 2.0f;

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
