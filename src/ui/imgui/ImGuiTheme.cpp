#include "ImGuiTheme.h"

#include <imgui.h>

namespace rf::ui {

void applyImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 3.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.ItemSpacing = ImVec2(7.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;

    ImVec4* colors = style.Colors;

    // Text

    colors[ImGuiCol_Text] =
        ImVec4(0.8f, 0.84f, 0.66f, 1.00f);

    colors[ImGuiCol_TextDisabled] =
        ImVec4(0.48f, 0.45f, 0.33f, 1.00f);

    // Backgrounds

    colors[ImGuiCol_WindowBg] =
        ImVec4(0.07f, 0.07f, 0.05f, 1.00f);

    colors[ImGuiCol_ChildBg] =
        ImVec4(0.14f, 0.13f, 0.09f, 1.00f);

    colors[ImGuiCol_PopupBg] =
        ImVec4(0.10f, 0.10f, 0.07f, 1.00f);

    // Borders

    colors[ImGuiCol_Border] =
        ImVec4(0.42f, 0.36f, 0.20f, 1.00f);

    colors[ImGuiCol_BorderShadow] =
        ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames

    colors[ImGuiCol_FrameBg] =
        ImVec4(0.19f, 0.17f, 0.11f, 1.00f);

    colors[ImGuiCol_FrameBgHovered] =
        ImVec4(0.27f, 0.25f, 0.15f, 1.00f);

    colors[ImGuiCol_FrameBgActive] =
        ImVec4(0.42f, 0.58f, 0.42f, 1.00f);

    colors[ImGuiCol_NavCursor] =
        ImVec4(0, 0, 0, 0);

    colors[ImGuiCol_NavWindowingHighlight] =
        ImVec4(0.44f, 0.64f, 0.26f, 0.45f);

    colors[ImGuiCol_TextSelectedBg] =
        ImVec4(0.28f, 0.42f, 0.18f, 0.80f);

    // Headers / Selection

    colors[ImGuiCol_Header]        = ImVec4(0.24f, 0.36f, 0.20f, 0.70f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.36f, 0.50f, 0.30f, 0.85f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(0.50f, 0.66f, 0.44f, 1.00f);

    // Buttons

    colors[ImGuiCol_Button] =
        ImVec4(0.20f, 0.18f, 0.11f, 1.00f);

    colors[ImGuiCol_ButtonHovered] =
        ImVec4(0.42f, 0.58f, 0.42f, 1.00f);

    colors[ImGuiCol_ButtonActive] =
        ImVec4(0.56f, 0.74f, 0.54f, 1.00f);

    // Checkboxes

    colors[ImGuiCol_CheckMark] =
        ImVec4(0.76f, 0.96f, 0.76f, 1.00f);

    // Sliders

    colors[ImGuiCol_SliderGrab] =
        ImVec4(0.56f, 0.76f, 0.52f, 1.00f);

    colors[ImGuiCol_SliderGrabActive] =
        ImVec4(0.74f, 0.94f, 0.70f, 1.00f);

    // Separators

    colors[ImGuiCol_Separator] =
        ImVec4(0.42f, 0.36f, 0.20f, 1.00f);

    // Tabs

    colors[ImGuiCol_Tab] =
        ImVec4(0.15f, 0.14f, 0.09f, 1.00f);

    colors[ImGuiCol_TabHovered] =
        ImVec4(0.42f, 0.58f, 0.42f, 1.00f);

    colors[ImGuiCol_TabActive] =
        ImVec4(0.32f, 0.44f, 0.30f, 1.00f);

    // Scrollbars

    colors[ImGuiCol_ScrollbarBg] =
        ImVec4(0.10f, 0.09f, 0.06f, 1.00f);

    colors[ImGuiCol_ScrollbarGrab] =
        ImVec4(0.34f, 0.30f, 0.18f, 1.00f);

    colors[ImGuiCol_ScrollbarGrabHovered] =
        ImVec4(0.48f, 0.62f, 0.42f, 1.00f);

    colors[ImGuiCol_ScrollbarGrabActive] =
        ImVec4(0.62f, 0.78f, 0.50f, 1.00f);
}

}
