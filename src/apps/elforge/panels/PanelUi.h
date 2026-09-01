#pragma once

#include <imgui.h>

namespace eld::elforge::ui {

inline void sectionHeader(const char* label) {
    ImGui::Spacing();
    ImGui::TextUnformatted(label);
    ImGui::Separator();
}

}
