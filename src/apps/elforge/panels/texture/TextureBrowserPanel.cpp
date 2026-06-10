#include "TextureBrowserPanel.h"

#include <imgui.h>
#include <cstdio>

namespace eldoria::apps::elforge {

void TextureBrowserPanel::render(
    CacheState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "TextureBrowserPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("TEXTURE BROWSER");
    ImGui::Separator();

    // Texture id input
    ImGui::TextUnformatted("Texture ID:");
    ImGui::PushItemWidth(-1.0f);
    ImGui::InputInt(
        "##texture_id",
        &state.textureInputId,
        1,
        10,
        ImGuiInputTextFlags_EnterReturnsTrue
    );
    ImGui::PopItemWidth();

    // Buttons row
    if (ImGui::Button("Load")) {
        if (state.textureInputId >= 0) {
            state.pendingTextureLoad = true;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset")) {
        state.textureInputId = 0;
        state.selectedTexture.reset();
        state.selectedTextureLoadError.reset();
    }

    ImGui::Separator();

    // Status
    if (state.selectedTextureLoadError.has_value()) {
        ImGui::TextColored(
            ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
            "%s",
            state.selectedTextureLoadError->c_str()
        );
    }

    if (state.selectedTexture.has_value()) {
        const auto& tex = *state.selectedTexture;
        ImGui::TextColored(
            ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
            "Loaded texture %d (%dx%d)",
            state.textureInputId,
            tex.metadata.width,
            tex.metadata.height
        );
    }

    ImGui::EndChild();
}

}
