#pragma once

#include <algorithm>
#include <string>

#include <imgui.h>

#include "ui/ElForgeTheme.h"
#include "ui/IconButton.h"
#include "viewport/ViewportWorkspaceLayout.h"

namespace eld::elforge::ui::workspace {

inline constexpr float HeaderHeight = 34.0f;

// 2D/media controls occupy real layout space beneath the canvas.
// The cards remain identical to the immersive 3D HUD cards.
inline constexpr float DockHeight = 120.0f;

inline constexpr float ToolRailWidth = 52.0f;
inline constexpr float ToolRailHeight = 244.0f;
inline constexpr float ToolRailX = 20.0f;
inline constexpr float ToolRailY = 20.0f;

inline constexpr ImVec2 ToolButtonSize{
    34.0f,
    34.0f
};

inline constexpr float ToolButtonGap = 2.0f;
inline constexpr float ToolDividerGap = 5.0f;


inline void renderIdentityHeader(
    const std::string& identity,
    const std::string& summary
) {
    ImGui::BeginGroup();

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        themePalette().primary
    );

    ImGui::TextUnformatted(
        identity.c_str()
    );

    ImGui::PopStyleColor();

    if (!summary.empty()) {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "%s",
            summary.c_str()
        );
    }

    ImGui::EndGroup();
}


inline void drawViewportAtmosphere(
    ImDrawList* drawList,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize
) {
    if (
        drawList == nullptr ||
        viewportSize.x <= 0.0f ||
        viewportSize.y <= 0.0f
    ) {
        return;
    }

    const ImVec2 viewportEnd{
        viewportPosition.x +
            viewportSize.x,
        viewportPosition.y +
            viewportSize.y
    };


    // This is intentionally the exact atmosphere that the
    // Animation workspace established as ElForge's visual DNA.
    const float topLightBottomY =
        viewportPosition.y +
        viewportSize.y *
            0.44f;

    drawList->AddRectFilledMultiColor(
        viewportPosition,
        ImVec2(
            viewportEnd.x,
            topLightBottomY
        ),
        IM_COL32(
            228,
            236,
            223,
            30
        ),
        IM_COL32(
            228,
            236,
            223,
            30
        ),
        IM_COL32(
            228,
            236,
            223,
            0
        ),
        IM_COL32(
            228,
            236,
            223,
            0
        )
    );


    const float sideVignetteWidth =
        viewportSize.x *
        0.18f;

    drawList->AddRectFilledMultiColor(
        viewportPosition,
        ImVec2(
            viewportPosition.x +
                sideVignetteWidth,
            viewportEnd.y
        ),
        IM_COL32(
            16,
            21,
            17,
            32
        ),
        IM_COL32(
            16,
            21,
            17,
            0
        ),
        IM_COL32(
            16,
            21,
            17,
            0
        ),
        IM_COL32(
            16,
            21,
            17,
            32
        )
    );

    drawList->AddRectFilledMultiColor(
        ImVec2(
            viewportEnd.x -
                sideVignetteWidth,
            viewportPosition.y
        ),
        viewportEnd,
        IM_COL32(
            16,
            21,
            17,
            0
        ),
        IM_COL32(
            16,
            21,
            17,
            32
        ),
        IM_COL32(
            16,
            21,
            17,
            32
        ),
        IM_COL32(
            16,
            21,
            17,
            0
        )
    );


    const float lowerFadeY =
        viewportPosition.y +
        viewportSize.y *
            0.52f;

    drawList->AddRectFilledMultiColor(
        ImVec2(
            viewportPosition.x,
            lowerFadeY
        ),
        viewportEnd,
        IM_COL32(
            12,
            17,
            13,
            0
        ),
        IM_COL32(
            12,
            17,
            13,
            0
        ),
        IM_COL32(
            12,
            17,
            13,
            164
        ),
        IM_COL32(
            12,
            17,
            13,
            164
        )
    );


    const float footerShadowY =
        viewportPosition.y +
        viewportSize.y *
            0.76f;

    drawList->AddRectFilledMultiColor(
        ImVec2(
            viewportPosition.x,
            footerShadowY
        ),
        viewportEnd,
        IM_COL32(
            7,
            10,
            8,
            0
        ),
        IM_COL32(
            7,
            10,
            8,
            0
        ),
        IM_COL32(
            7,
            10,
            8,
            92
        ),
        IM_COL32(
            7,
            10,
            8,
            92
        )
    );
}


inline void beginBottomHud(
    const char* id,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize
) {
    const ImVec2 bottomHudPosition{
        viewportPosition.x,
        viewportPosition.y +
            viewportSize.y -
            viewport_workspace::
                BottomHeight
    };


    ImDrawList* parentDrawList =
        ImGui::GetWindowDrawList();

    parentDrawList->AddRectFilledMultiColor(
        ImVec2(
            viewportPosition.x,
            bottomHudPosition.y -
                viewport_workspace::
                    FadeHeight
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
        id,
        ImVec2(
            viewportSize.x,
            viewport_workspace::
                BottomHeight
        ),
        false,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    ImGui::PopStyleVar();
}


inline void endBottomHud() {
    ImGui::EndChild();
    ImGui::PopStyleColor();
}


inline void beginDockedHud(
    const char* id,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    ImGui::SetCursorScreenPos(
        controlsPosition
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
        themePalette().hudBackground
    );

    ImGui::BeginChild(
        id,
        controlsSize,
        true,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    ImGui::PopStyleVar();
}


inline void endDockedHud() {
    ImGui::EndChild();
    ImGui::PopStyleColor();
}


inline float dockedCardY(
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    return
        controlsPosition.y +
        std::max(
            (
                controlsSize.y -
                viewport_workspace::CardHeight
            ) *
                0.5f,
            0.0f
        );
}


inline float cardY(
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize
) {
    return
        viewportPosition.y +
        viewportSize.y -
        viewport_workspace::
            CardHeight -
        viewport_workspace::
            BottomInset;
}


inline void beginCard(
    const char* id,
    const ImVec2& position,
    const ImVec2& size
) {
    ImGui::SetCursorScreenPos(
        position
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
        themePalette().hudBackground
    );

    ImGui::BeginChild(
        id,
        size,
        true,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );
}


inline void endCard() {
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}


inline void centeredText(
    const std::string& text,
    float y,
    bool disabled = false
) {
    const float width =
        ImGui::CalcTextSize(
            text.c_str()
        ).x;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    ImGui::GetWindowWidth() -
                        width
                ) *
                    0.5f,
                8.0f
            ),
            y
        )
    );

    if (disabled) {
        ImGui::TextDisabled(
            "%s",
            text.c_str()
        );
    }
    else {
        ImGui::TextUnformatted(
            text.c_str()
        );
    }
}


inline bool pillButton(
    const char* id,
    const char* label,
    bool active,
    const ImVec2& size
) {
    const auto& palette =
        themePalette();

    ImGui::PushID(
        id
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        size.y *
            0.5f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameBorderSize,
        0.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        active
            ? ImVec4(
                  palette.primary.x,
                  palette.primary.y,
                  palette.primary.z,
                  0.22f
              )
            : ImVec4(
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
            active
                ? 0.32f
                : 0.14f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ImVec4(
            palette.primary.x,
            palette.primary.y,
            palette.primary.z,
            0.40f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        active
            ? palette.primary
            : palette.text
    );

    const bool clicked =
        ImGui::Button(
            label,
            size
        );

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    ImGui::PopID();

    return clicked;
}


inline bool navigationButton(
    const char* id,
    Icon icon,
    const char* tooltip,
    const ImVec2& size =
        ImVec2(
            26.0f,
            28.0f
        )
) {
    const auto& palette =
        themePalette();

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        size.y *
            0.5f
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
        iconButton(
            id,
            icon,
            tooltip,
            size
        );

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    return clicked;
}


inline void cardDivider(
    float y = 46.0f,
    float inset = 12.0f
) {
    const ImVec2 panelMin =
        ImGui::GetWindowPos();

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(
            panelMin.x +
                inset,
            panelMin.y +
                y
        ),
        ImVec2(
            panelMin.x +
                ImGui::GetWindowWidth() -
                inset,
            panelMin.y +
                y
        ),
        ImGui::GetColorU32(
            ImGuiCol_Border
        ),
        1.0f
    );
}


inline void beginToolRail(
    const char* id,
    const ImVec2& viewportPosition
) {
    ImGui::SetCursorScreenPos(
        ImVec2(
            viewportPosition.x +
                ToolRailX,
            viewportPosition.y +
                ToolRailY
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ChildRounding,
        7.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(
            1.0f,
            2.0f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        themePalette().hudBackground
    );

    ImGui::BeginChild(
        id,
        ImVec2(
            ToolRailWidth,
            ToolRailHeight
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    ImGui::SetCursorPosY(
        5.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(
            0.0f,
            ToolButtonGap
        )
    );
}


inline void endToolRail() {
    ImGui::PopStyleVar();

    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}


inline bool toolButton(
    const char* id,
    Icon icon,
    const char* tooltip,
    bool active = false
) {
    const auto& palette =
        themePalette();

    ImGui::SetCursorPosX(
        (
            ImGui::GetWindowWidth() -
            ToolButtonSize.x
        ) *
            0.5f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        ToolButtonSize.y *
            0.5f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameBorderSize,
        0.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        active
            ? ImVec4(
                  palette.primary.x,
                  palette.primary.y,
                  palette.primary.z,
                  0.22f
              )
            : ImVec4(
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
            active
                ? 0.32f
                : 0.14f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ImVec4(
            palette.primary.x,
            palette.primary.y,
            palette.primary.z,
            0.40f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        active
            ? palette.primary
            : palette.text
    );

    const bool clicked =
        iconButton(
            id,
            icon,
            tooltip,
            ToolButtonSize
        );

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    return clicked;
}


inline void toolDivider() {
    ImGui::SetCursorPosY(
        ImGui::GetCursorPosY() +
            ToolDividerGap *
                0.5f
    );

    constexpr float DividerInset =
        10.0f;

    const ImVec2 windowPosition =
        ImGui::GetWindowPos();

    const float y =
        ImGui::GetCursorScreenPos().y;

    const auto& palette =
        themePalette();

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(
            windowPosition.x +
                DividerInset,
            y
        ),
        ImVec2(
            windowPosition.x +
                ImGui::GetWindowWidth() -
                DividerInset,
            y
        ),
        ImGui::ColorConvertFloat4ToU32(
            ImVec4(
                palette.border.x,
                palette.border.y,
                palette.border.z,
                0.65f
            )
        ),
        1.0f
    );

    ImGui::Dummy(
        ImVec2(
            1.0f,
            ToolDividerGap
        )
    );
}

}
