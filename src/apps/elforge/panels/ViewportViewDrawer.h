#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>

#include <imgui.h>

#include "../CacheExplorerState.h"

#include "InterfaceViewPanel.h"
#include "ModelViewPanel.h"
#include "TextureViewPanel.h"

namespace eld::elforge {

enum class ViewportViewKind {
    None,
    Map,
    Interface,
    Npc,
    Location,
    SpotAnimation,
    Model,
    Texture
};

struct ViewportDrawerLayout {
    float drawerHeight = 30.0f;
    float resizeHandleHeight = 0.0f;
    float minimumHeight = 30.0f;
    float maximumHeight = 30.0f;
};

class ViewportViewDrawer {
public:
    static constexpr float CollapsedHeight =
        30.0f;

    ViewportViewKind kindFor(
        const CacheExplorerState& state
    ) const {
        if (state.activeMap.has_value()) {
            return ViewportViewKind::Map;
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

    void update(
        CacheExplorerState& state,
        ViewportViewKind kind
    ) {
        modelViewPanel_.update(
            state,
            kind ==
                ViewportViewKind::Model ||
            kind ==
                ViewportViewKind::Npc ||
            kind ==
                ViewportViewKind::Location ||
            kind ==
                ViewportViewKind::SpotAnimation
        );
    }

    ViewportDrawerLayout updateLayout(
        float availableHeight
    ) {
        constexpr float MinimumViewportHeight =
            80.0f;

        const float maximumHeight =
            std::max(
                CollapsedHeight,
                availableHeight -
                    MinimumViewportHeight
            );

        const float minimumHeight =
            std::min(
                150.0f,
                maximumHeight
            );

        preferredHeight_ =
            std::clamp(
                preferredHeight_,
                minimumHeight,
                maximumHeight
            );

        const float targetHeight =
            open_
                ? preferredHeight_
                : CollapsedHeight;

        const float deltaTime =
            std::max(
                ImGui::GetIO().DeltaTime,
                0.0f
            );

        const float animation =
            1.0f -
            std::exp(
                -14.0f *
                deltaTime
            );

        animatedHeight_ +=
            (
                targetHeight -
                animatedHeight_
            ) *
            animation;

        if (
            std::abs(
                animatedHeight_ -
                targetHeight
            ) <
            0.5f
        ) {
            animatedHeight_ =
                targetHeight;
        }

        return {
            std::clamp(
                animatedHeight_,
                CollapsedHeight,
                maximumHeight
            ),
            open_
                ? 5.0f
                : 0.0f,
            minimumHeight,
            maximumHeight
        };
    }

    void renderResizeHandle(
        const ViewportDrawerLayout& layout
    ) {
        if (!open_) {
            return;
        }

        ImGui::InvisibleButton(
            "##ViewportViewResize",
            ImVec2(
                -1.0f,
                layout.resizeHandleHeight
            )
        );

        if (
            ImGui::IsItemHovered() ||
            ImGui::IsItemActive()
        ) {
            ImGui::SetMouseCursor(
                ImGuiMouseCursor_ResizeNS
            );
        }

        if (ImGui::IsItemActive()) {
            preferredHeight_ -=
                ImGui::GetIO().MouseDelta.y;

            preferredHeight_ =
                std::clamp(
                    preferredHeight_,
                    layout.minimumHeight,
                    layout.maximumHeight
                );

            animatedHeight_ =
                preferredHeight_;
        }
    }

    // ELFORGE_NPC_ANIMATION_DRAWER_V1
    void render(
        CacheExplorerState& state,
        ViewportViewKind kind,
        float drawerHeight,
        const std::function<void()>&
            renderAnimationControls
    ) {
        ImGui::BeginChild(
            "ViewportViewDrawer",
            ImVec2(
                0.0f,
                drawerHeight
            ),
            true
        );

        const char* title =
            titleFor(
                kind
            );

        const std::string label =
            std::string(
                open_
                    ? "v  "
                    : "^  "
            ) +
            title;

        if (
            ImGui::Selectable(
                label.c_str(),
                false,
                0,
                ImVec2(
                    0.0f,
                    22.0f
                )
            )
        ) {
            open_ =
                !open_;
        }

        if (
            drawerHeight >
                CollapsedHeight +
                    20.0f
        ) {
            ImGui::Separator();

            renderActivePanel(
                state,
                kind,
                renderAnimationControls
            );
        }

        ImGui::EndChild();
    }

    const InterfaceViewOptions&
    interfaceOptions() const {
        return
            interfaceViewPanel_.options();
    }

    const ModelViewOptions&
    modelOptions() const {
        return
            modelViewPanel_.options();
    }

    const TextureViewOptions&
    textureOptions() const {
        return
            textureViewPanel_.options();
    }

private:
    static const char* titleFor(
        ViewportViewKind kind
    ) {
        switch (kind) {
            case ViewportViewKind::Map:
                return "MAP VIEW";

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

    void renderActivePanel(
        CacheExplorerState& state,
        ViewportViewKind kind,
        const std::function<void()>&
            renderAnimationControls
    ) {
        switch (kind) {
            case ViewportViewKind::Map:
                if (state.activeMap.has_value()) {
                    const MapPreview& map =
                        *state.activeMap;

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

            case ViewportViewKind::Interface:
                interfaceViewPanel_.render(
                    true
                );
                break;

            case ViewportViewKind::Npc:
                if (renderAnimationControls) {
                    renderAnimationControls();
                }

                ImGui::Spacing();

                modelViewPanel_.render(
                    state,
                    true
                );
                break;

            case ViewportViewKind::Location:
            case ViewportViewKind::SpotAnimation:
                if (renderAnimationControls) {
                    renderAnimationControls();
                }

                ImGui::Spacing();

                modelViewPanel_.render(
                    state,
                    true
                );
                break;

            case ViewportViewKind::Model:
                if (renderAnimationControls) {
                    renderAnimationControls();
                }

                ImGui::Spacing();

                modelViewPanel_.render(
                    state,
                    true
                );
                break;

            case ViewportViewKind::Texture:
                textureViewPanel_.render(
                    true
                );
                break;

            case ViewportViewKind::None:
            default:
                ImGui::TextDisabled(
                    "Select a map, interface, NPC, object, SpotAnim, model, or texture."
                );
                break;
        }
    }

    InterfaceViewPanel interfaceViewPanel_;
    ModelViewPanel modelViewPanel_;
    TextureViewPanel textureViewPanel_;

    bool open_ = false;
    float preferredHeight_ = 300.0f;
    float animatedHeight_ = CollapsedHeight;
};

}
