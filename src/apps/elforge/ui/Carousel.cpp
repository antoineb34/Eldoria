#include "ui/Carousel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "ui/IconButton.h"

namespace eld::elforge::ui {

CarouselResult carousel(
    const char* id,
    const char* label,
    const std::string& value,
    std::size_t index,
    std::size_t count,
    float width,
    const char* countTooltip
) {
    CarouselResult result;

    ImGui::PushID(id);
    ImGui::BeginGroup();

    ImGui::AlignTextToFramePadding();

    ImGui::TextDisabled(
        "%s",
        label
    );

    ImGui::SameLine();

    const float labelWidth =
        ImGui::CalcTextSize(label).x;

    constexpr float ArrowWidth =
        28.0f;

    constexpr float CounterWidth =
        48.0f;

    const float spacing =
        ImGui::GetStyle().ItemSpacing.x;

    result.previous =
        iconButton(
            "##Previous",
            Icon::ChevronLeft,
            "Previous",
            ImVec2(
                ArrowWidth,
                28.0f
            )
        );

    ImGui::SameLine();

    const float valueWidth =
        std::max(
            90.0f,
            width -
                labelWidth -
                ArrowWidth * 2.0f -
                CounterWidth -
                spacing * 5.0f
        );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImGui::GetStyleColorVec4(
            ImGuiCol_FrameBg
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImGui::GetStyleColorVec4(
            ImGuiCol_FrameBgHovered
        )
    );

    ImGui::Button(
        "##Value",
        ImVec2(
            valueWidth,
            28.0f
        )
    );

    ImGui::PopStyleColor(2);

    const ImVec2 minimum =
        ImGui::GetItemRectMin();

    const ImVec2 maximum =
        ImGui::GetItemRectMax();

    std::string display =
        value;

    const float textWidth =
        maximum.x -
        minimum.x -
        14.0f;

    while (
        display.size() > 4 &&
        ImGui::CalcTextSize(
            display.c_str()
        ).x >
        textWidth
    ) {
        display.pop_back();
    }

    if (display != value) {
        if (display.size() > 3) {
            display.resize(
                display.size() - 3
            );
        }

        display += "...";

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
    }

    const ImVec2 textSize =
        ImGui::CalcTextSize(
            display.c_str()
        );

    ImGui::GetWindowDrawList()->AddText(
        ImVec2(
            minimum.x +
                (
                    maximum.x -
                    minimum.x -
                    textSize.x
                ) *
                0.5f,
            minimum.y +
                (
                    maximum.y -
                    minimum.y -
                    textSize.y
                ) *
                0.5f
        ),
        ImGui::GetColorU32(
            ImGuiCol_Text
        ),
        display.c_str()
    );

    ImGui::SameLine();

    result.next =
        iconButton(
            "##Next",
            Icon::ChevronRight,
            "Next",
            ImVec2(
                ArrowWidth,
                28.0f
            )
        );

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();

    ImGui::TextDisabled(
        "%zu/%zu",
        count > 0
            ? index + 1
            : 0,
        count
    );

    ImGui::EndGroup();
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
