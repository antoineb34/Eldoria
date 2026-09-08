#include "ui/ElForgeTheme.h"
#include "views/animation/AnimationViewPanel.h"
#include "views/animation/AnimationHudLayout.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"
#include "ui/IconButton.h"

namespace eld::elforge {

namespace {

std::string useLabel(
    const AnimationUse& use
) {
    const std::string name =
        !use.sourceName.empty()
            ? use.sourceName
            : use.source;

    return
        name +
        " - " +
        std::to_string(
            use.sourceId
        );
}


std::string sequenceLabel(
    const AnimationSequenceReference& sequence,
    const std::string& role
) {
    return
        std::to_string(
            sequence.sequenceId
        ) +
        "  ·  " +
        (
            role.empty()
                ? "Animation"
                : role
        ) +
        "  ·  " +
        std::to_string(
            sequence.durationMilliseconds
        ) +
        "ms";
}


enum class ContextRowIcon {
    Sequence,
    Usage
};


struct ContextRowResult {
    bool previous = false;
    bool next = false;
};


ContextRowResult contextRow(
    const char* id,
    ContextRowIcon icon,
    const char* tooltip,
    const std::string& value,
    std::size_t index,
    std::size_t count,
    float width,
    const char* countTooltip
) {
    ContextRowResult result;

    constexpr float RowInsetX =
        12.0f;

    constexpr float IconSlotWidth =
        24.0f;

    constexpr float ArrowWidth =
        26.0f;

    constexpr float RowHeight =
        28.0f;


    ImGui::SetCursorPosX(
        RowInsetX
    );

    ImGui::PushID(
        id
    );


    // --------------------------------------------------------
    // Semantic icon.
    // --------------------------------------------------------

    const ImVec2 iconPosition =
        ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(
        "##ContextIcon",
        ImVec2(
            IconSlotWidth,
            RowHeight
        )
    );

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    const ImU32 iconColor =
        ImGui::ColorConvertFloat4ToU32(
            ui::themePalette().primary
        );

    const ImVec2 center{
        iconPosition.x +
            IconSlotWidth * 0.5f,
        iconPosition.y +
            RowHeight * 0.5f
    };


    if (
        icon ==
        ContextRowIcon::Sequence
    ) {
        // Tiny filmstrip / timeline icon.
        const ImVec2 minimum{
            center.x - 8.0f,
            center.y - 6.0f
        };

        const ImVec2 maximum{
            center.x + 8.0f,
            center.y + 6.0f
        };

        drawList->AddRect(
            minimum,
            maximum,
            iconColor,
            2.0f,
            0,
            1.4f
        );

        drawList->AddLine(
            ImVec2(
                minimum.x + 5.0f,
                minimum.y
            ),
            ImVec2(
                minimum.x + 5.0f,
                maximum.y
            ),
            iconColor,
            1.0f
        );

        drawList->AddLine(
            ImVec2(
                minimum.x + 11.0f,
                minimum.y
            ),
            ImVec2(
                minimum.x + 11.0f,
                maximum.y
            ),
            iconColor,
            1.0f
        );
    }
    else {
        // Relationship / usage icon.
        const ImVec2 left{
            center.x - 5.0f,
            center.y
        };

        const ImVec2 right{
            center.x + 5.0f,
            center.y
        };

        drawList->AddLine(
            left,
            right,
            iconColor,
            1.6f
        );

        drawList->AddCircle(
            left,
            3.5f,
            iconColor,
            0,
            1.5f
        );

        drawList->AddCircle(
            right,
            3.5f,
            iconColor,
            0,
            1.5f
        );
    }


    if (
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "%s",
            tooltip
        );
    }


    ImGui::SameLine(
        0.0f,
        4.0f
    );


    // --------------------------------------------------------
    // Modern borderless previous/next buttons.
    // --------------------------------------------------------

    const auto navigationButton =
        [](const char* buttonId,
           ui::Icon buttonIcon,
           const char* buttonTooltip) {
            const auto& palette =
                ui::themePalette();

            ImGui::PushStyleVar(
                ImGuiStyleVar_FrameRounding,
                13.0f
            );

            ImGui::PushStyleVar(
                ImGuiStyleVar_FrameBorderSize,
                0.0f
            );

            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f
                )
            );

            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImVec4(
                    palette.primary.x,
                    palette.primary.y,
                    palette.primary.z,
                    0.16f
                )
            );

            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImVec4(
                    palette.primary.x,
                    palette.primary.y,
                    palette.primary.z,
                    0.27f
                )
            );

            const bool clicked =
                ui::iconButton(
                    buttonId,
                    buttonIcon,
                    buttonTooltip,
                    ImVec2(
                        ArrowWidth,
                        RowHeight
                    )
                );

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(2);

            return clicked;
        };


    result.previous =
        navigationButton(
            "Previous",
            ui::Icon::ChevronLeft,
            "Previous"
        );


    ImGui::SameLine(
        0.0f,
        5.0f
    );


    const std::string countText =
        std::to_string(
            index + 1
        ) +
        "/" +
        std::to_string(
            count
        );

    const float countWidth =
        ImGui::CalcTextSize(
            countText.c_str()
        ).x;

    const float valueWidth =
        std::max(
            110.0f,
            width -
                IconSlotWidth -
                ArrowWidth * 2.0f -
                countWidth -
                52.0f
        );


    // --------------------------------------------------------
    // Value.
    // --------------------------------------------------------

    ImGui::BeginChild(
        "##ContextValue",
        ImVec2(
            valueWidth,
            RowHeight
        ),
        false,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBackground
    );

    ImGui::SetCursorPosY(
        4.0f
    );

    ImGui::TextUnformatted(
        value.c_str()
    );

    if (
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "%s",
            value.c_str()
        );
    }

    ImGui::EndChild();


    ImGui::SameLine(
        0.0f,
        5.0f
    );


    result.next =
        navigationButton(
            "Next",
            ui::Icon::ChevronRight,
            "Next"
        );


    ImGui::SameLine(
        0.0f,
        6.0f
    );


    ImGui::TextDisabled(
        "%s",
        countText.c_str()
    );

    if (
        countTooltip != nullptr &&
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "%s",
            countTooltip
        );
    }


    ImGui::PopID();

    return result;
}

}


void AnimationViewPanel::render(
    CacheExplorerState& state,
    const std::function<void()>&
        renderPlayerHud
) const {
    if (!state.activeAnimation.has_value()) {
        return;
    }

    const AnimationInspection& info =
        *state.activeAnimation;

    AnimationViewState& view =
        state.animationView;

    std::vector<std::size_t> sequenceIndices;

    for (
        std::size_t sequenceIndex = 0;
        sequenceIndex < info.sequences.size();
        ++sequenceIndex
    ) {
        const std::uint16_t sequenceId =
            info.sequences[
                sequenceIndex
            ].sequenceId;

        const bool previewable =
            std::any_of(
                view.previewUseIndices.begin(),
                view.previewUseIndices.end(),
                [&info, sequenceId](
                    std::size_t useIndex
                ) {
                    return
                        useIndex <
                            info.uses.size() &&
                        info.uses[
                            useIndex
                        ].sequenceId ==
                            sequenceId;
                }
            );

        if (previewable) {
            sequenceIndices.push_back(
                sequenceIndex
            );
        }
    }

    if (sequenceIndices.empty()) {
        return;
    }

    view.selectedSequenceIndex =
        std::min(
            view.selectedSequenceIndex,
            sequenceIndices.size() - 1
        );

    const auto currentSequence =
        [&]()
            -> const AnimationSequenceReference& {
            return info.sequences[
                sequenceIndices[
                    view.selectedSequenceIndex
                ]
            ];
        };

    const auto findFirstUse =
        [&](
            std::uint16_t sequenceId
        ) -> std::size_t {
            for (
                std::size_t previewIndex = 0;
                previewIndex <
                    view.previewUseIndices.size();
                ++previewIndex
            ) {
                const std::size_t useIndex =
                    view.previewUseIndices[
                        previewIndex
                    ];

                if (
                    useIndex < info.uses.size() &&
                    info.uses[
                        useIndex
                    ].sequenceId ==
                        sequenceId
                ) {
                    return previewIndex;
                }
            }

            return
                AnimationViewState::
                    NoActivePreview;
        };

    const auto syncUse =
        [&]() {
            const std::uint16_t sequenceId =
                currentSequence().sequenceId;

            bool valid = false;

            if (
                view.selectedPreviewUseIndex <
                view.previewUseIndices.size()
            ) {
                const std::size_t useIndex =
                    view.previewUseIndices[
                        view.selectedPreviewUseIndex
                    ];

                valid =
                    useIndex < info.uses.size() &&
                    info.uses[
                        useIndex
                    ].sequenceId ==
                        sequenceId;
            }

            if (!valid) {
                const std::size_t first =
                    findFirstUse(
                        sequenceId
                    );

                if (
                    first !=
                    AnimationViewState::
                        NoActivePreview
                ) {
                    view.selectedPreviewUseIndex =
                        first;
                }
            }
        };

    syncUse();

    const auto currentUsageRole =
        [&]() -> std::string {
            if (
                view.selectedPreviewUseIndex >=
                view.previewUseIndices.size()
            ) {
                return {};
            }

            const std::size_t useIndex =
                view.previewUseIndices[
                    view.selectedPreviewUseIndex
                ];

            if (
                useIndex >=
                info.uses.size()
            ) {
                return {};
            }

            const AnimationUse& use =
                info.uses[
                    useIndex
                ];

            if (
                use.sequenceId !=
                currentSequence().sequenceId
            ) {
                return {};
            }

            return use.role;
        };

    const auto buildUses =
        [&]() {
            std::vector<std::size_t> result;

            const std::uint16_t sequenceId =
                currentSequence().sequenceId;

            for (
                std::size_t previewIndex = 0;
                previewIndex <
                    view.previewUseIndices.size();
                ++previewIndex
            ) {
                const std::size_t useIndex =
                    view.previewUseIndices[
                        previewIndex
                    ];

                if (
                    useIndex < info.uses.size() &&
                    info.uses[
                        useIndex
                    ].sequenceId ==
                        sequenceId
                ) {
                    result.push_back(
                        previewIndex
                    );
                }
            }

            return result;
        };

    const ImVec2 viewportPosition{
        static_cast<float>(
            state.viewportX
        ),
        static_cast<float>(
            state.viewportY
        )
    };

    const ImVec2 viewportSize{
        static_cast<float>(
            state.viewportWidth
        ),
        static_cast<float>(
            state.viewportHeight
        )
    };

    // --------------------------------------------------------
    // BOTTOM HUD SCRIM
    //
    // Dark translucent band behind the lower controls so the
    // timeline + cards read better over bright viewport colors.
    // --------------------------------------------------------
    {
        // ----------------------------------------------------
        // BOTTOM HUD
        //
        // The timeline and all bottom animation controls now
        // live inside this child. The timeline therefore uses
        // the HUD's top edge as its actual layout anchor rather
        // than reproducing that position with magic numbers.
        // ----------------------------------------------------

        constexpr float BottomHudHeight =
            animation_hud::BottomHeight;

        constexpr float BottomHudFadeHeight =
            animation_hud::FadeHeight;

        const ImVec2 bottomHudPosition{
            viewportPosition.x,
            viewportPosition.y +
                viewportSize.y -
                BottomHudHeight
        };


        // Soft transition from viewport into the HUD.
        ImDrawList* parentDrawList =
            ImGui::GetWindowDrawList();

        parentDrawList->AddRectFilledMultiColor(
            ImVec2(
                viewportPosition.x,
                bottomHudPosition.y -
                    BottomHudFadeHeight
            ),
            ImVec2(
                viewportPosition.x +
                    viewportSize.x,
                bottomHudPosition.y
            ),
            IM_COL32(
                6,
                8,
                10,
                0
            ),
            IM_COL32(
                6,
                8,
                10,
                0
            ),
            IM_COL32(
                6,
                8,
                10,
                150
            ),
            IM_COL32(
                6,
                8,
                10,
                150
            )
        );


        ImGui::SetCursorScreenPos(
            bottomHudPosition
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(
                0.0f,
                0.0f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            IM_COL32(
                6,
                8,
                10,
                150
            )
        );

        ImGui::BeginChild(
            "##AnimationBottomHud",
            ImVec2(
                viewportSize.x,
                BottomHudHeight
            ),
            false,
            ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        );

        ImGui::PopStyleVar();


        // Timeline + playback + frame inspector are owned by
        // the animation player side but are now rendered as
        // children of this HUD.
        if (renderPlayerHud) {
            renderPlayerHud();
        }
    }

    constexpr float BottomInset =
        animation_hud::BottomInset;

    constexpr float cardHeight =
        animation_hud::CardHeight;

    const animation_hud::BottomRow hudLayout =
        animation_hud::bottomRow(
            viewportSize.x
        );

    const float cardWidth =
        hudLayout.contextWidth;

    const float cardX =
        hudLayout.contextX;

    ImGui::SetCursorScreenPos(
        ImVec2(
            viewportPosition.x +
                cardX,
            viewportPosition.y +
                viewportSize.y -
                cardHeight -
                BottomInset
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ChildRounding,
        8.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(
            0.0f,
            0.0f
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(
            8.0f,
            6.0f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        ui::themePalette().hudBackground
    );

    ImGui::BeginChild(
        "##AnimationContextCard",
        ImVec2(
            cardWidth,
            cardHeight
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    const float contextWidth =
        cardWidth;

    ImGui::SetCursorPos(
        ImVec2(
            0.0f,
            0.0f
        )
    );

    ImGui::BeginChild(
        "##AnimationSequenceContextHalf",
        ImVec2(
            contextWidth,
            45.0f
        ),
        false,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBackground
    );

    ImGui::SetCursorPosY(
        10.0f
    );

    const std::string sequenceCountTooltip =
        std::to_string(
            sequenceIndices.size()
        ) +
        (
            sequenceIndices.size() == 1
                ? " compatible sequence"
                : " compatible sequences"
        );

    const ContextRowResult sequenceResult =
        contextRow(
            "Sequence",
            ContextRowIcon::Sequence,
            "SEQUENCE",
            sequenceLabel(
                currentSequence(),
                currentUsageRole()
            ),
            view.selectedSequenceIndex,
            sequenceIndices.size(),
            contextWidth,
            sequenceCountTooltip.c_str()
        );

    if (sequenceResult.previous) {
        view.selectedSequenceIndex =
            view.selectedSequenceIndex > 0
                ? view.selectedSequenceIndex - 1
                : sequenceIndices.size() - 1;

        syncUse();
    }

    if (sequenceResult.next) {
        view.selectedSequenceIndex =
            (
                view.selectedSequenceIndex +
                1
            ) %
            sequenceIndices.size();

        syncUse();
    }

    ImGui::EndChild();

    ImGui::SetCursorPos(
        ImVec2(
            0.0f,
            46.0f
        )
    );

    ImGui::BeginChild(
        "##AnimationUsageContextHalf",
        ImVec2(
            contextWidth,
            45.0f
        ),
        false,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBackground
    );

    ImGui::SetCursorPosY(
        10.0f
    );

    std::vector<std::size_t> uses =
        buildUses();

    if (!uses.empty()) {
        auto selected =
            std::find(
                uses.begin(),
                uses.end(),
                view.selectedPreviewUseIndex
            );

        if (selected == uses.end()) {
            view.selectedPreviewUseIndex =
                uses.front();

            selected =
                uses.begin();
        }

        std::size_t usePosition =
            static_cast<std::size_t>(
                selected -
                uses.begin()
            );

        const std::size_t useIndex =
            view.previewUseIndices[
                view.selectedPreviewUseIndex
            ];

        const std::string useCountTooltip =
            std::to_string(
                uses.size()
            ) +
            (
                uses.size() == 1
                    ? " known usage for this sequence"
                    : " known usages for this sequence"
            );

        const ContextRowResult useResult =
            contextRow(
                "Usage",
                ContextRowIcon::Usage,
                "USAGE",
                useLabel(
                    info.uses[
                        useIndex
                    ]
                ),
                usePosition,
                uses.size(),
                contextWidth,
                useCountTooltip.c_str()
            );

        if (useResult.previous) {
            usePosition =
                usePosition > 0
                    ? usePosition - 1
                    : uses.size() - 1;

            view.selectedPreviewUseIndex =
                uses[
                    usePosition
                ];
        }

        if (useResult.next) {
            usePosition =
                (
                    usePosition +
                    1
                ) %
                uses.size();

            view.selectedPreviewUseIndex =
                uses[
                    usePosition
                ];
        }
    }

    ImGui::EndChild();

    // Split Sequence and Usage without introducing another
    // heavy panel or visible label.
    ImDrawList* contextDrawList =
        ImGui::GetWindowDrawList();

    const ImVec2 contextPanelMin =
        ImGui::GetWindowPos();

    const float dividerY =
        contextPanelMin.y +
            46.0f;

    contextDrawList->AddLine(
        ImVec2(
            contextPanelMin.x +
                12.0f,
            dividerY
        ),
        ImVec2(
            contextPanelMin.x +
                cardWidth -
                12.0f,
            dividerY
        ),
        ImGui::GetColorU32(
            ImGuiCol_Border
        ),
        1.0f
    );

    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    // End the shared bottom animation HUD.
    ImGui::EndChild();
    ImGui::PopStyleColor();


}

}
