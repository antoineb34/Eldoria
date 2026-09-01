#include "ui/ElForgeTheme.h"
#include "views/animation/AnimationViewOverlay.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include <imgui.h>

#include "dump/AnimationDumper.h"
#include "explorer/CacheExplorerState.h"
#include "ui/IconButton.h"

namespace eld::elforge {

namespace {

void propertyRow(
    const char* label,
    const std::string& value
) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);

    ImGui::TextDisabled(
        "%s",
        label
    );

    ImGui::TableSetColumnIndex(1);

    ImGui::TextUnformatted(
        value.c_str()
    );
}

}


void AnimationViewOverlay::renderHeader(
    CacheExplorerState& state
) const {
    if (!state.activeAnimation.has_value()) {
        return;
    }

    const AnimationInspection& info =
        *state.activeAnimation;

    const eld::animation::Animation& animation =
        info.animation;

    // --------------------------------------------------------
    // LEFT: asset identity
    // --------------------------------------------------------

    ImGui::BeginGroup();

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        ui::themePalette().primary
    );

    ImGui::Text(
        "ANIMATION %u",
        static_cast<unsigned int>(
            animation.id
        )
    );

    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::TextDisabled(
        "%zu frames  ·  %zu slots",
        animation.asset.frames.size(),
        animation.asset.skeleton.slots.size()
    );

    ImGui::EndGroup();

    // --------------------------------------------------------
    // RIGHT: workspace actions
    // --------------------------------------------------------

    constexpr float ButtonSize =
        28.0f;

    constexpr float ButtonGap =
        5.0f;

    const float actionWidth =
        ButtonSize * 2.0f +
        ButtonGap;

    const float rightX =
        ImGui::GetWindowContentRegionMax().x -
        actionWidth;

    ImGui::SameLine();

    if (
        rightX >
        ImGui::GetCursorPosX()
    ) {
        ImGui::SetCursorPosX(
            rightX
        );
    }

    if (
        ui::iconButton(
            "##AnimationInfo",
            ui::Icon::Info,
            "Animation details",
            ImVec2(
                ButtonSize,
                ButtonSize
            )
        )
    ) {
        ImGui::OpenPopup(
            "##AnimationDetailsPopup"
        );
    }

    ImGui::SameLine(
        0.0f,
        ButtonGap
    );

    if (
        ui::iconButton(
            "##AnimationDump",
            ui::Icon::Download,
            "Dump full animation asset",
            ImVec2(
                ButtonSize,
                ButtonSize
            )
        )
    ) {
        const auto dumpPath =
            defaultAnimationDumpPath(
                animation.id
            );

        std::string error;

        if (
            dumpAnimation(
                info,
                dumpPath,
                error
            )
        ) {
            state.animationDumpStatus =
                "Dumped: " +
                dumpPath.string();
        }
        else {
            state.animationDumpStatus =
                "Dump failed: " +
                error;
        }
    }

    // --------------------------------------------------------
    // DETAILS CARD
    //
    // Popup windows are rendered above the viewport and have
    // proper interaction/z-order.
    // --------------------------------------------------------

    // Anchor the details card to the animation workspace,
    // never to the mouse/button. The Explorer lives outside
    // this header window and must never be covered by it.
    constexpr float DetailsWidth =
        270.0f;

    const ImVec2 headerPosition =
        ImGui::GetWindowPos();

    const ImVec2 headerSize =
        ImGui::GetWindowSize();

    const float detailsX =
        std::max(
            headerPosition.x + 8.0f,
            headerPosition.x +
                headerSize.x -
                DetailsWidth -
                8.0f
        );

    const float detailsY =
        headerPosition.y +
        headerSize.y +
        4.0f;

    ImGui::SetNextWindowPos(
        ImVec2(
            detailsX,
            detailsY
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            DetailsWidth,
            0.0f
        ),
        ImGuiCond_Appearing
    );

    if (
        ImGui::BeginPopup(
            "##AnimationDetailsPopup",
            ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings
        )
    ) {
        ImGui::TextUnformatted(
            "ANIMATION DETAILS"
        );

        ImGui::Separator();
        ImGui::Spacing();

        std::uint16_t minimumFrame = 0;
        std::uint16_t maximumFrame = 0;

        if (!animation.asset.frames.empty()) {
            minimumFrame =
                animation.asset.frames.front().id;

            maximumFrame =
                minimumFrame;

            for (
                const eld::animation::AnimationFrame& frame :
                animation.asset.frames
            ) {
                minimumFrame =
                    std::min(
                        minimumFrame,
                        frame.id
                    );

                maximumFrame =
                    std::max(
                        maximumFrame,
                        frame.id
                    );
            }
        }

        if (
            ImGui::BeginTable(
                "##AnimationDetailsTable",
                2,
                ImGuiTableFlags_SizingStretchProp
            )
        ) {
            propertyRow(
                "ID",
                std::to_string(
                    animation.id
                )
            );

            propertyRow(
                "Frames",
                std::to_string(
                    animation.asset.frames.size()
                )
            );

            propertyRow(
                "Skeleton slots",
                std::to_string(
                    animation.asset.skeleton.slots.size()
                )
            );

            propertyRow(
                "Raw bytes",
                std::to_string(
                    animation.file.bytes.size()
                )
            );

            if (!animation.asset.frames.empty()) {
                propertyRow(
                    "Frame IDs",
                    std::to_string(
                        minimumFrame
                    ) +
                    " - " +
                    std::to_string(
                        maximumFrame
                    )
                );
            }

            propertyRow(
                "Sequences",
                std::to_string(
                    info.sequences.size()
                )
            );

            propertyRow(
                "Known uses",
                std::to_string(
                    info.uses.size()
                )
            );

            ImGui::EndTable();
        }

        if (!state.animationDumpStatus.empty()) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextWrapped(
                "%s",
                state.animationDumpStatus.c_str()
            );
        }

        ImGui::EndPopup();
    }
}

}
