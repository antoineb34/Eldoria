#include "views/animation/AnimationViewPanel.h"
#include "viewport/ViewportControlsPanel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"

namespace eld::elforge {

ViewportViewKind ViewportControlsPanel::kindFor(
    const CacheExplorerState& state
) const {
    if (state.activeMap.has_value()) {
        return ViewportViewKind::Map;
    }
    if (state.activeMidi.has_value()) {
        return ViewportViewKind::Midi;
    }
    if (state.activeAnimation.has_value()) {
        return ViewportViewKind::Animation;
    }
    if (state.activeInterface.has_value()) {
        return ViewportViewKind::Interface;
    }
    if (state.activeTexture.has_value()) {
        return ViewportViewKind::Texture;
    }
    if (
        state.activeLocation.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Location;
    }
    if (
        state.activeSpotAnimation.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::SpotAnimation;
    }
    if (
        state.activeNpc.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Npc;
    }
    if (
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Model;
    }

    return ViewportViewKind::None;
}

void ViewportControlsPanel::update(
    CacheExplorerState& state,
    ViewportViewKind kind
) {
    if (kind != lastKind_) {
        if (kind == ViewportViewKind::Animation) {
            open_ = true;

            // Animation's sequence/preview/playback controls
            // are primary interaction, not a hidden drawer.
            preferredHeight_ = 108.0f;
        }
        else {
            if (
                lastKind_ ==
                ViewportViewKind::Animation
            ) {
                preferredHeight_ = 175.0f;
            }

            if (kind == ViewportViewKind::Midi) {
                open_ = true;
            }
        }

        lastKind_ = kind;
    }

    modelViewPanel_.update(
        state,
        kind == ViewportViewKind::Model ||
            kind == ViewportViewKind::Npc ||
            kind == ViewportViewKind::Location ||
            kind == ViewportViewKind::SpotAnimation
    );
}

ViewportControlsLayout ViewportControlsPanel::updateLayout(
    float availableHeight
) {
    if (
        lastKind_ ==
        ViewportViewKind::Animation
    ) {
        constexpr float ToolbarHeight =
            86.0f;

        const float height =
            std::min(
                ToolbarHeight,
                std::max(
                    CollapsedHeight,
                    availableHeight
                )
            );

        return {
            height,
            0.0f,
            height,
            height
        };
    }


    constexpr float MinimumViewportHeight = 80.0f;

    const float maximumHeight = std::max(
        CollapsedHeight,
        availableHeight - MinimumViewportHeight
    );

    const float minimumHeight = std::min(
        84.0f,
        maximumHeight
    );

    preferredHeight_ = std::clamp(
        preferredHeight_,
        minimumHeight,
        maximumHeight
    );

    const float targetHeight =
        open_ ? preferredHeight_ : CollapsedHeight;

    const float deltaTime =
        std::max(ImGui::GetIO().DeltaTime, 0.0f);

    const float animation =
        1.0f - std::exp(-14.0f * deltaTime);

    animatedHeight_ +=
        (targetHeight - animatedHeight_) * animation;

    if (std::abs(animatedHeight_ - targetHeight) < 0.5f) {
        animatedHeight_ = targetHeight;
    }

    return {
        std::clamp(
            animatedHeight_,
            CollapsedHeight,
            maximumHeight
        ),
        open_ ? 5.0f : 0.0f,
        minimumHeight,
        maximumHeight
    };
}

void ViewportControlsPanel::renderResizeHandle(
    const ViewportControlsLayout& layout
) {
    if (
        !open_ ||
        layout.resizeHandleHeight <= 0.0f
    ) {
        return;
    }

    ImGui::InvisibleButton(
        "##ViewportViewResize",
        ImVec2(-1.0f, layout.resizeHandleHeight)
    );

    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }

    if (ImGui::IsItemActive()) {
        preferredHeight_ -= ImGui::GetIO().MouseDelta.y;
        preferredHeight_ = std::clamp(
            preferredHeight_,
            layout.minimumHeight,
            layout.maximumHeight
        );
        animatedHeight_ = preferredHeight_;
    }
}

void ViewportControlsPanel::render(
    CacheExplorerState& state,
    ViewportViewKind kind,
    float controlsHeight,
    const std::function<void()>& renderAnimationControls,
    const std::function<void()>& renderMidiControls
) {
    ImGui::BeginChild(
        "ViewportControlsPanel",
        ImVec2(0.0f, controlsHeight),
        false
    );

    // Animation uses this area as a permanent contextual
    // toolbar. Do not hide primary playback behind a drawer.
    if (kind == ViewportViewKind::Animation) {
        renderActivePanel(
            state,
            kind,
            renderAnimationControls,
            renderMidiControls
        );

        ImGui::EndChild();
        return;
    }

    const std::string label =
        std::string(open_ ? "▾  " : "▴  ") +
        titleFor(kind);

    if (
        ImGui::Selectable(
            label.c_str(),
            false,
            0,
            ImVec2(0.0f, 24.0f)
        )
    ) {
        open_ = !open_;
    }

    if (controlsHeight > CollapsedHeight + 20.0f) {
        ImGui::Separator();
        renderActivePanel(
            state,
            kind,
            renderAnimationControls,
            renderMidiControls
        );
    }

    ImGui::EndChild();
}

const InterfaceViewOptions&
ViewportControlsPanel::interfaceOptions() const {
    return interfaceViewPanel_.options();
}

const ModelViewOptions&
ViewportControlsPanel::modelOptions() const {
    return modelViewPanel_.options();
}

const TextureViewOptions&
ViewportControlsPanel::textureOptions() const {
    return textureViewPanel_.options();
}

const char* ViewportControlsPanel::titleFor(
    ViewportViewKind kind
) {
    switch (kind) {
        case ViewportViewKind::Map:
            return "MAP VIEW";
        case ViewportViewKind::Midi:
            return "MIDI VIEW";
        case ViewportViewKind::Animation:
            return "ANIMATION VIEW";
        case ViewportViewKind::Interface:
            return "INTERFACE VIEW";
        case ViewportViewKind::Npc:
            return "NPC VIEW";
        case ViewportViewKind::Location:
            return "OBJECT VIEW";
        case ViewportViewKind::SpotAnimation:
            return "SPOT ANIMATION VIEW";
        case ViewportViewKind::Model:
            return "MODEL VIEW";
        case ViewportViewKind::Texture:
            return "TEXTURE VIEW";
        case ViewportViewKind::None:
        default:
            return "VIEW";
    }
}

void ViewportControlsPanel::renderActivePanel(
    CacheExplorerState& state,
    ViewportViewKind kind,
    const std::function<void()>& renderAnimationControls,
    const std::function<void()>& renderMidiControls
) {
    switch (kind) {
        case ViewportViewKind::Map:
            if (state.activeMap.has_value()) {
                const MapViewState& map = *state.activeMap;

                ImGui::Text(
                    "Region %u (%d,%d)",
                    static_cast<unsigned int>(
                        map.indexEntry.regionId
                    ),
                    map.indexEntry.regionX(),
                    map.indexEntry.regionY()
                );

                ImGui::Text(
                    "World base: %d, %d",
                    map.centerRegion.worldBaseX(),
                    map.centerRegion.worldBaseY()
                );

                ImGui::Text(
                    "Terrain file: %u | Object file: %u",
                    static_cast<unsigned int>(
                        map.indexEntry.terrainFileId
                    ),
                    static_cast<unsigned int>(
                        map.indexEntry.objectFileId
                    )
                );

                ImGui::Separator();

                ImGui::Text(
                    "Objects: %zu | model instances: %zu | variants: %zu",
                    map.stats.locPlacements,
                    map.stats.locModelInstances,
                    map.stats.locModelVariants
                );

                ImGui::Text(
                    "Terrain triangles: %zu | object triangles: %zu",
                    map.stats.terrainTriangles,
                    map.stats.locTriangles
                );

                ImGui::Text(
                    "Build: %.2f ms | neighborhood: %zu/9",
                    map.stats.buildMilliseconds,
                    map.stats.neighborhoodRegions
                );

                ImGui::Text(
                    "Camera: yaw %.2f pitch %.2f distance %.1f",
                    state.mapYaw,
                    state.mapPitch,
                    state.mapDistance
                );
            }
            break;

        case ViewportViewKind::Midi:
            if (renderMidiControls) {
                renderMidiControls();
            }
            break;

        case ViewportViewKind::Animation:
            AnimationViewPanel{}.render(
                state,
                renderAnimationControls
            );
            break;

        case ViewportViewKind::Interface:
            interfaceViewPanel_.render(true);
            break;

        case ViewportViewKind::Npc:
        case ViewportViewKind::Location:
        case ViewportViewKind::SpotAnimation:
        case ViewportViewKind::Model:
            if (renderAnimationControls) {
                renderAnimationControls();
            }
            ImGui::Spacing();
            modelViewPanel_.render(state, true);
            break;

        case ViewportViewKind::Texture:
            textureViewPanel_.render(true);
            break;

        case ViewportViewKind::None:
        default:
            ImGui::TextDisabled(
                "Select a map, MIDI, animation, interface, NPC, object, SpotAnim, model, or texture."
            );
            break;
    }
}

}
