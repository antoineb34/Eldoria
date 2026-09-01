#include "explorer/CacheExplorer.h"

#include <cmath>
#include <exception>
#include <filesystem>
#include <string>

#include <imgui.h>

namespace eld::elforge {

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      mapLoader_(cache_),
      midiRepository_(
          cache_.open(
              eld::cache::IndexId::Midi
          )
      ),
      animationRepository_(
          cache_.open(
              eld::cache::IndexId::Animations
          )
      ),
      animationFrameIndex_(
          animationRepository_
      ),
      animationPlayer_(
          animationFrameIndex_
      ),
      animationPresentationCatalog_(
          "content/animation_bindings.csv"
      ),
      textureRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          )
      ),
      modelRepository_(
          cache_.open(
              eld::cache::IndexId::Models
          )
      ),
      titleSpriteRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),
      mediaSpriteRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          4
      ),

      titleJpegRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),
      titleFontRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),
      definitionRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          2
      ),
      floorRepository_(
          definitionRepository_.get(
              "flo"
          )
      ),
      identityKitRepository_(
          definitionRepository_.get(
              "idk"
          )
      ),
      locationRepository_(
          definitionRepository_.get(
              "loc"
          )
      ),
      npcRepository_(
          definitionRepository_.get(
              "npc"
          )
      ),
      itemRepository_(
          definitionRepository_.get(
              "obj"
          )
      ),
      sequenceRepository_(
          definitionRepository_.get(
              "seq"
          )
      ),
      spotAnimationRepository_(
          definitionRepository_.get(
              "spotanim"
          )
      ),
      varpRepository_(
          definitionRepository_.get("varp")
      ),
      varbitRepository_(
          definitionRepository_.get("varbit")
      ),
      parameterRepository_(
          definitionRepository_.get("param")
      ),
      messageRepository_(
          definitionRepository_.get("mes")
      ),
      messageAnimationRepository_(
          definitionRepository_.get("mesanim")
      ),
      interfaceRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          3
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
        state_.midiPlaybackStatus =
            midiPlayer_.statusMessage();
    }
    else {
        state_.midiPlaybackStatus =
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
                animationFrameIndex_,
                sequenceRepository_,
                npcRepository_,
                locationRepository_,
                spotAnimationRepository_,
                itemRepository_,
                interfaceRepository_,
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
        interfaceRepository_,
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

    ImGui::TextUnformatted(
        "ElForge"
    );

    ImGui::Separator();

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
        height,
        midiPlayer_,
        [this]() {
            renderAnimationControls();
        }
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
