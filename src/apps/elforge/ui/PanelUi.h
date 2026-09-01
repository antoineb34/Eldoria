#pragma once

#include <imgui.h>

#include "ui/ElForgeTheme.h"

namespace eld::elforge::ui {

inline void panelTitle(
    const char* label
) {
    ImGui::PushStyleColor(
        ImGuiCol_Text,
        themePalette().primary
    );

    ImGui::TextUnformatted(
        label
    );

    ImGui::PopStyleColor();
}


inline void sectionHeader(
    const char* label
) {
    ImGui::Spacing();

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        themePalette().tertiary
    );

    ImGui::TextUnformatted(
        label
    );

    ImGui::PopStyleColor();

    ImGui::Separator();
}

}
