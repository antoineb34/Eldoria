#include "ui/ElForgeTheme.h"
#include "explorer/CacheExplorer.h"
#include "dump/AssetDumper.h"
#include "ui/IconButton.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <string>

#include <imgui.h>

namespace eld::elforge {

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      mapRepository_(cache_),
      midiRepository_(
          cache_
      ),
      animationRepository_(
          cache_
      ),
      animationFrameTable_(
          animationRepository_
      ),
      animationPlayer_(
          animationFrameTable_
      ),
      animationPresentationCatalog_(
          "content/animation_bindings.csv"
      ),
      textureRepository_(
          cache_
      ),
      modelRepository_(
          cache_
      ),
      titleSpriteRepository_(
          cache_,
          1
      ),
      mediaSpriteRepository_(
          cache_,
          4
      ),

      titleImageRepository_(
          cache_,
          1
      ),
      titleFontRepository_(
          cache_,
          1
      ),
      definitionArchive_(
          eld::archive::load(
              cache_.open(
                  eld::cache::IndexId::Config
              ),
              2
          )
      ),
      floorRepository_(
          cache_
      ),
      identityKitRepository_(
          cache_
      ),
      locationRepository_(
          cache_
      ),
      npcRepository_(
          cache_
      ),
      itemRepository_(
          cache_
      ),
      sequenceRepository_(
          cache_
      ),
      spotAnimationRepository_(
          cache_
      ),
      varpRepository_(
          cache_
      ),
      varbitRepository_(
          cache_
      ),
      parameterRepository_(
          cache_
      ),
      messageRepository_(
          cache_
      ),
      messageAnimationRepository_(
          cache_
      ),
      widgetRepository_(
          cache_
      ),
      graphicsResources_(
          modelRepository_,
          textureRepository_
      ) {
}

void CacheExplorer::shutdown() {
    midiPlayer_.shutdown();
    viewportPanel_.shutdown();
}

bool CacheExplorer::initialize() {
    state_.viewportCameraPivot = {
        0.0f,
        0.0f,
        0.0f
    };

    state_.viewportCameraDistance =
        650.0f;

    state_.camera.rotation = {
        0.42f,
        -0.55f,
        0.0f
    };

    const float pitch =
        state_.camera.rotation.x;

    const float yaw =
        state_.camera.rotation.y;

    const eld::math::Vec3 forward{
        std::cos(pitch) * std::sin(yaw),
        -std::sin(pitch),
        std::cos(pitch) * std::cos(yaw)
    };

    state_.camera.position =
        state_.viewportCameraPivot -
        forward *
            state_.viewportCameraDistance;

    state_.camera.verticalFov =
        1.04719755f;

    state_.camera.nearPlane = 1.0f;
    state_.camera.farPlane = 10000.0f;

    state_.camera.viewportWidth = 1;
    state_.camera.viewportHeight = 1;

    state_.rootNode =
        treeBuilder_.build(
            cache_,
            textureRepository_.listIds()
        );

    lastAnimationUpdateMs_ =
        SDL_GetTicks();

    if (!midiPlayer_.initialize()) {
        state_.midiView.playbackStatus =
            midiPlayer_.statusMessage();
    }
    else {
        state_.midiView.playbackStatus =
            "MIDI playback ready";
    }

    return true;
}

void CacheExplorer::handleEvent(
    const SDL_Event& event
) {
    if (
        !placeActionTargetOnClick_ ||
        !state_.activeNpc.has_value() ||
        event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT
    ) {
        return;
    }

    const float mouseX = event.button.x;
    const float mouseY = event.button.y;

    const float viewportLeft =
        static_cast<float>(state_.viewportX);

    const float viewportTop =
        static_cast<float>(state_.viewportY);

    const float viewportRight =
        viewportLeft +
        static_cast<float>(state_.viewportWidth);

    const float viewportBottom =
        viewportTop +
        static_cast<float>(state_.viewportHeight);

    if (
        mouseX < viewportLeft ||
        mouseX >= viewportRight ||
        mouseY < viewportTop ||
        mouseY >= viewportBottom
    ) {
        return;
    }

    placeActionTargetFromViewport(
        mouseX,
        mouseY
    );
}

void CacheExplorer::update() {
    const std::uint64_t now =
        SDL_GetTicks();

    const std::uint64_t delta =
        lastAnimationUpdateMs_ == 0
            ? 0
            : now - lastAnimationUpdateMs_;

    lastAnimationUpdateMs_ =
        now;

    if (
        state_.selection.key !=
        lastSelectedKey_
    ) {
        lastSelectedKey_ =
            state_.selection.key;

        handleSelectionChanged();

        lastAnimationUpdateMs_ =
            now;

        return;
    }

    if (state_.animationExportAllRequested) {
        state_.animationExportAllRequested = false;

        try {
            const AnimationInspector relations(
                animationRepository_,
                animationFrameTable_,
        sequenceRepository_,
                npcRepository_,
                locationRepository_,
                spotAnimationRepository_,
                itemRepository_,
                widgetRepository_,
                animationPresentationCatalog_
            );

            const std::filesystem::path path =
                defaultAnimationRelationsExportPath();

            std::string error;

            if (
                exportAllAnimationInspections(
                    relations,
                    path,
                    error
                )
            ) {
                state_.animationExportStatus =
                    "Exported: " +
                    path.string();
            }
            else {
                state_.animationExportStatus =
                    "Export failed: " +
                    error;
            }
        }
        catch (const std::exception& exception) {
            state_.animationExportStatus =
                std::string("Export failed: ") +
                exception.what();
        }
    }

    if (
        state_.activeAnimation.has_value() &&
        !state_.animationView.previewUseIndices.empty() &&
        state_.animationView.selectedPreviewUseIndex !=
            state_.animationView.activePreviewUseIndex
    ) {
        activateAnimationPreviewUse(
            state_.animationView.selectedPreviewUseIndex
        );
    }

    if (
        animationSource_.has_value() &&
        animationPlayer_.update(
            delta
        )
    ) {
        rebuildAnimationFrame();
    }

    updateNpcActionEffects(
        delta
    );

    // An active semantic/composed action owns facing while this lock is on.
    // Moving the NPC or moving the target therefore recomputes yaw every
    // frame instead of taking a one-time snapshot at action start.
    if (
        lockNpcFacingToActionTarget_ &&
        activeNpcAction_.has_value() &&
        state_.activeNpc.has_value()
    ) {
        faceNpcTowardActionTarget();
    }

}

void CacheExplorer::prepareViewport(
    SDL_Renderer* renderer
) {
    viewportPanel_.prepareViewport(
        renderer,
        state_,
        graphicsResources_
    );
}

void CacheExplorer::renderViewport(
    SDL_Renderer* renderer
) {
    viewportPanel_.renderViewport(
        renderer,
        state_,
        graphicsResources_,
        widgetRepository_,
        mediaSpriteRepository_
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

    const ImGuiWindowFlags shellFlags =
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

    const bool animationWorkspace =
        state_.activeAnimation.has_value();

    // --------------------------------------------------------
    // Application bar
    // --------------------------------------------------------

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        ui::themePalette().primary
    );

    ImGui::TextUnformatted(
        "ELFORGE"
    );

    ImGui::PopStyleColor();

    if (!state_.selection.label.empty()) {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "/  %s",
            state_.selection.label.c_str()
        );
    }

    const char* explorerButton =
        explorerPanelOpen_
            ? "Explorer -"
            : "Explorer +";

    const bool hasSelection =
        !state_.selection.key.empty();

    constexpr float ActionButtonSize =
        28.0f;

    constexpr float ActionButtonGap =
        5.0f;

    float rightControlsWidth =
        ImGui::CalcTextSize(
            explorerButton
        ).x +
        20.0f;

    if (hasSelection) {
        rightControlsWidth +=
            ActionButtonSize * 2.0f +
            ActionButtonGap +
            10.0f;
    }

    const float rightX =
        ImGui::GetWindowContentRegionMax().x -
        rightControlsWidth;

    if (
        rightX >
        ImGui::GetCursorPosX()
    ) {
        ImGui::SameLine(
            rightX
        );
    }
    else {
        ImGui::SameLine();
    }

    if (hasSelection) {
        if (
            ui::iconButton(
                "##AssetDetails",
                ui::Icon::Info,
                "Asset details",
                ImVec2(
                    ActionButtonSize,
                    ActionButtonSize
                )
            )
        ) {
            ImGui::OpenPopup(
                "##AssetDetailsPopup"
            );
        }

        ImGui::SameLine(
            0.0f,
            ActionButtonGap
        );

        if (
            ui::iconButton(
                "##AssetDump",
                ui::Icon::Download,
                "Dump full asset",
                ImVec2(
                    ActionButtonSize,
                    ActionButtonSize
                )
            )
        ) {
            const std::filesystem::path path =
                defaultAssetDumpPath(
                    state_
                );

            std::string error;

            if (
                dumpActiveAsset(
                    state_,
                    path,
                    error
                )
            ) {
                state_.assetDumpStatus =
                    "Dumped: " +
                    path.string();
            }
            else {
                state_.assetDumpStatus =
                    "Dump failed: " +
                    error;
            }

            ImGui::OpenPopup(
                "##AssetDetailsPopup"
            );
        }

        ImGui::SameLine(
            0.0f,
            10.0f
        );
    }

    if (
        ImGui::SmallButton(
            explorerButton
        )
    ) {
        explorerPanelOpen_ =
            !explorerPanelOpen_;
    }

    // Details belongs to the asset workspace, never to the
    // Explorer/navigation column. Keep the popup dynamically
    // clamped to the current workspace even if Explorer is
    // opened or closed while Details is visible.
    constexpr float DetailsExplorerWidth =
        252.0f;

    constexpr float DetailsInset =
        8.0f;

    const ImVec2 shellPosition =
        ImGui::GetWindowPos();

    const ImVec2 contentMinimum =
        ImGui::GetWindowContentRegionMin();

    const ImVec2 contentMaximum =
        ImGui::GetWindowContentRegionMax();

    const float detailsWorkspaceLeft =
        shellPosition.x +
        contentMinimum.x;

    float detailsWorkspaceRight =
        shellPosition.x +
        contentMaximum.x;

    if (explorerPanelOpen_) {
        detailsWorkspaceRight -=
            DetailsExplorerWidth +
            ImGui::GetStyle().ItemSpacing.x;
    }

    const float detailsWorkspaceWidth =
        std::max(
            detailsWorkspaceRight -
                detailsWorkspaceLeft,
            1.0f
        );

    const float detailsWidth =
        std::max(
            std::min(
                560.0f,
                detailsWorkspaceWidth -
                    DetailsInset * 2.0f
            ),
            100.0f
        );

    const float detailsHeight =
        std::max(
            std::min(
                viewport->WorkSize.y -
                    90.0f,
                640.0f
            ),
            120.0f
        );

    const float detailsX =
        std::max(
            detailsWorkspaceLeft +
                DetailsInset,
            detailsWorkspaceRight -
                detailsWidth -
                DetailsInset
        );

    const float detailsY =
        ImGui::GetCursorScreenPos().y +
        ImGui::GetFrameHeightWithSpacing();

    ImGui::SetNextWindowPos(
        ImVec2(
            detailsX,
            detailsY
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            detailsWidth,
            detailsHeight
        ),
        ImGuiCond_Always
    );

    if (
        ImGui::BeginPopup(
            "##AssetDetailsPopup",
            ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoMove
        )
    ) {
        const ImVec2 detailsSize =
            ImGui::GetContentRegionAvail();

        detailsPanel_.render(
            state_,
            detailsSize.x,
            detailsSize.y
        );

        ImGui::EndPopup();
    }

    ImGui::Separator();

    // --------------------------------------------------------
    // Workspace allocation
    // --------------------------------------------------------

    constexpr float ExplorerWidth =
        252.0f;

    const float spacing =
        ImGui::GetStyle().ItemSpacing.x;

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const float height =
        available.y;

    float workspaceWidth =
        available.x;

    if (explorerPanelOpen_) {
        workspaceWidth -=
            ExplorerWidth +
            spacing;
    }

    workspaceWidth =
        std::max(
            workspaceWidth,
            120.0f
        );

    // Main asset workspace always starts at the left.
    viewportPanel_.render(
        state_,
        workspaceWidth,
        height,
        midiPlayer_,
        [this]() {
            renderAnimationControls();
        }
    );

    // Navigation always lives on the far-right.
    if (explorerPanelOpen_) {
        ImGui::SameLine();

        treePanel_.render(
            state_,
            ExplorerWidth,
            height
        );
    }

    ImGui::End();

}

}
