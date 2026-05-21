#include "ImGuiTheme.h"

#include <imgui.h>

namespace rf::ui {

    void applyImGuiTheme() {

        ImGuiStyle& style =
            ImGui::GetStyle();

        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(10.0f, 8.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
        style.IndentSpacing = 16.0f;
        style.ScrollbarSize = 12.0f;

        ImVec4* colors =
            style.Colors;

        colors[ImGuiCol_Text] =
            ImVec4(0.90f, 0.92f, 0.93f, 1.00f);

        colors[ImGuiCol_TextDisabled] =
            ImVec4(0.55f, 0.58f, 0.60f, 1.00f);

        colors[ImGuiCol_WindowBg] =
            ImVec4(0.12f, 0.13f, 0.14f, 0.30f);

        colors[ImGuiCol_ChildBg] =
            ImVec4(0.15f, 0.16f, 0.18f, 0.30f);

        colors[ImGuiCol_PopupBg] =
            ImVec4(0.12f, 0.13f, 0.14f, 1.00f);

        colors[ImGuiCol_Border] =
            ImVec4(0.24f, 0.26f, 0.29f, 1.00f);

        colors[ImGuiCol_FrameBg] =
            ImVec4(0.18f, 0.20f, 0.22f, 1.00f);

        colors[ImGuiCol_FrameBgHovered] =
            ImVec4(0.24f, 0.27f, 0.30f, 1.00f);

        colors[ImGuiCol_FrameBgActive] =
            ImVec4(0.30f, 0.34f, 0.37f, 1.00f);

        colors[ImGuiCol_TitleBg] =
            ImVec4(0.10f, 0.11f, 0.12f, 1.00f);

        colors[ImGuiCol_TitleBgActive] =
            ImVec4(0.15f, 0.16f, 0.18f, 1.00f);

        colors[ImGuiCol_Button] =
            ImVec4(0.20f, 0.22f, 0.24f, 1.00f);

        colors[ImGuiCol_ButtonHovered] =
            ImVec4(0.28f, 0.31f, 0.34f, 1.00f);

        colors[ImGuiCol_ButtonActive] =
            ImVec4(0.35f, 0.39f, 0.43f, 1.00f);

        colors[ImGuiCol_Header] =
            ImVec4(0.20f, 0.22f, 0.24f, 1.00f);

        colors[ImGuiCol_HeaderHovered] =
            ImVec4(0.28f, 0.31f, 0.34f, 1.00f);

        colors[ImGuiCol_HeaderActive] =
            ImVec4(0.35f, 0.39f, 0.43f, 1.00f);

        colors[ImGuiCol_CheckMark] =
            ImVec4(0.78f, 0.82f, 0.88f, 1.00f);

        colors[ImGuiCol_SliderGrab] =
            ImVec4(0.50f, 0.54f, 0.58f, 1.00f);

        colors[ImGuiCol_SliderGrabActive] =
            ImVec4(0.70f, 0.74f, 0.78f, 1.00f);

        colors[ImGuiCol_Separator] =
            ImVec4(0.24f, 0.26f, 0.29f, 1.00f);

        colors[ImGuiCol_Tab] =
            ImVec4(0.15f, 0.16f, 0.18f, 1.00f);

        colors[ImGuiCol_TabHovered] =
            ImVec4(0.28f, 0.31f, 0.34f, 1.00f);

        colors[ImGuiCol_TabActive] =
            ImVec4(0.22f, 0.25f, 0.28f, 1.00f);
    }
}
