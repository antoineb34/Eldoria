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
        if (
            kind == ViewportViewKind::Midi ||
            kind == ViewportViewKind::Animation
        ) {
            open_ = true;
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
    constexpr float MinimumViewportHeight = 80.0f;

    const float maximumHeight = std::max(
        CollapsedHeight,
        availableHeight - MinimumViewportHeight
    );

    const float minimumHeight = std::min(
        150.0f,
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
    if (!open_) {
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
        true
    );

    const std::string label =
        std::string(open_ ? "v  " : "^  ") + titleFor(kind);

    if (
        ImGui::Selectable(
            label.c_str(),
            false,
            0,
            ImVec2(0.0f, 22.0f)
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
            renderAnimationArchiveView(state);
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

const char* ViewportControlsPanel::transformTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0:
            return "Pivot";
        case 1:
            return "Translate";
        case 2:
            return "Rotate";
        case 3:
            return "Scale";
        case 4:
            return "Unknown4";
        case 5:
            return "Alpha";
        default:
            return "Unknown";
    }
}

void ViewportControlsPanel::renderAnimationArchiveView(
    CacheExplorerState& state
) {
    if (!state.activeAnimation.has_value()) {
        return;
    }

    const AnimationInspection& info = *state.activeAnimation;
    const eld::animation::Animation& animation = info.animation;
    const std::size_t frameCount = animation.asset.frames.size();

    ImGui::Text(
        "Animation %u",
        static_cast<unsigned int>(animation.id)
    );
    ImGui::SameLine();
    ImGui::TextDisabled(
        "| %zu frames | %zu skeleton slots",
        frameCount,
        animation.asset.skeleton.slots.size()
    );
    ImGui::TextDisabled(
        "%zu referenced sequences | %zu known uses",
        info.sequences.size(),
        info.uses.size()
    );

    if (frameCount == 0) {
        ImGui::Spacing();
        ImGui::TextDisabled(
            "This animation contains no decoded frames."
        );
        return;
    }

    state.activeAnimationFrameIndex = std::min(
        state.activeAnimationFrameIndex,
        frameCount - 1
    );

    ImGui::Spacing();

    if (ImGui::Button("< Frame")) {
        if (state.activeAnimationFrameIndex > 0) {
            --state.activeAnimationFrameIndex;
        }
        else {
            state.activeAnimationFrameIndex = frameCount - 1;
        }
    }

    ImGui::SameLine();

    int frameIndex = static_cast<int>(
        state.activeAnimationFrameIndex
    );

    ImGui::SetNextItemWidth(
        std::clamp(
            ImGui::GetContentRegionAvail().x - 110.0f,
            80.0f,
            360.0f
        )
    );

    if (
        ImGui::SliderInt(
            "##AnimationFrameIndex",
            &frameIndex,
            0,
            static_cast<int>(frameCount - 1),
            "Frame %d"
        )
    ) {
        state.activeAnimationFrameIndex =
            static_cast<std::size_t>(frameIndex);
    }

    ImGui::SameLine();

    if (ImGui::Button("Frame >")) {
        state.activeAnimationFrameIndex =
            (state.activeAnimationFrameIndex + 1) % frameCount;
    }

    const eld::animation::AnimationFrame& frame =
        animation.asset.frames[state.activeAnimationFrameIndex];

    std::array<std::size_t, 7> transformCounts{};

    for (
        const eld::animation::FrameTransform& transform :
        frame.transforms
    ) {
        std::uint8_t type = 6;

        if (transform.slot < animation.asset.skeleton.slots.size()) {
            const std::uint8_t sourceType =
                animation.asset.skeleton.slots[transform.slot].type;
            type = sourceType <= 5 ? sourceType : 6;
        }

        ++transformCounts[type];
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text(
        "Global frame %u | delay %u | %zu transforms",
        static_cast<unsigned int>(frame.id),
        static_cast<unsigned int>(frame.delay),
        frame.transforms.size()
    );

    ImGui::Text(
        "Pivot %zu  Translate %zu  Rotate %zu  Scale %zu  Alpha %zu",
        transformCounts[0],
        transformCounts[1],
        transformCounts[2],
        transformCounts[3],
        transformCounts[5]
    );

    if (transformCounts[4] != 0 || transformCounts[6] != 0) {
        ImGui::TextDisabled(
            "Unknown4 %zu  Other %zu",
            transformCounts[4],
            transformCounts[6]
        );
    }

    if (ImGui::CollapsingHeader("Selected frame transforms")) {
        if (frame.transforms.empty()) {
            ImGui::TextDisabled(
                "No explicit transforms in this frame."
            );
        }

        for (
            const eld::animation::FrameTransform& transform :
            frame.transforms
        ) {
            std::uint8_t type = 255;

            if (transform.slot < animation.asset.skeleton.slots.size()) {
                type =
                    animation.asset.skeleton.slots[transform.slot].type;
            }

            ImGui::Text(
                "slot %u  %-9s  x=%d y=%d z=%d",
                static_cast<unsigned int>(transform.slot),
                transformTypeName(type),
                transform.x,
                transform.y,
                transform.z
            );
        }
    }

    if (ImGui::CollapsingHeader("Referenced sequences")) {
        if (info.sequences.empty()) {
            ImGui::TextDisabled("No sequence references found.");
        }

        for (
            const AnimationSequenceReference& sequence :
            info.sequences
        ) {
            const std::size_t matching =
                sequence.matchingPrimaryFrames +
                sequence.matchingSecondaryFrames;

            ImGui::Text(
                "Sequence %u  %zu/%zu refs",
                static_cast<unsigned int>(sequence.sequenceId),
                matching,
                sequence.totalFrameReferences
            );
        }
    }

    if (ImGui::CollapsingHeader("Known uses")) {
        if (info.uses.empty()) {
            ImGui::TextDisabled(
                "No known cache/content users found."
            );
        }

        for (const AnimationUse& use : info.uses) {
            std::string label =
                use.source + " " + std::to_string(use.sourceId);

            if (!use.sourceName.empty()) {
                label += " - " + use.sourceName;
            }

            label +=
                " - " + use.role +
                " - Seq " + std::to_string(use.sequenceId);

            label += " [" + use.provenance + "]";

            ImGui::TextWrapped("%s", label.c_str());
        }
    }
}

}
