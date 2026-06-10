#include "TextureMetadataPanel.h"

#include <imgui.h>

namespace eldoria::apps::elforge {

void TextureMetadataPanel::render(
    CacheState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "TextureMetadataPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("TEXTURE METADATA");
    ImGui::Separator();

    if (!state.selectedTexture.has_value()) {
        if (state.selectedTextureLoadError.has_value()) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                "%s",
                state.selectedTextureLoadError->c_str()
            );
        } else {
            ImGui::TextUnformatted("No texture selected.");
        }
        ImGui::EndChild();
        return;
    }

    const auto& tex = *state.selectedTexture;
    const auto& meta = tex.metadata;

    ImGui::TextColored(
        ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
        "Status: Loaded"
    );

    ImGui::Separator();

    ImGui::Text("Texture ID: %d", state.textureInputId);
    ImGui::Separator();

    ImGui::TextUnformatted("DIMENSIONS");
    ImGui::Text("  Width: %d", meta.width);
    ImGui::Text("  Height: %d", meta.height);
    ImGui::Text("  Canvas Width: %d", meta.canvasWidth);
    ImGui::Text("  Canvas Height: %d", meta.canvasHeight);
    ImGui::Separator();

    ImGui::TextUnformatted("OFFSET");
    ImGui::Text("  X Offset: %d", meta.xOffset);
    ImGui::Text("  Y Offset: %d", meta.yOffset);
    ImGui::Separator();

    ImGui::Text("Type: %d", meta.type);
    ImGui::Separator();

    ImGui::TextUnformatted("PALETTE");
    ImGui::Text("  Colors: %d", static_cast<int>(tex.palette.colors.size()));
    ImGui::Separator();

    ImGui::TextUnformatted("PIXELS");
    ImGui::Text("  Count: %d", static_cast<int>(tex.pixels.size()));
    int opaqueCount = 0;
    int transparentCount = 0;
    for (const auto& px : tex.pixels) {
        if (px.a == 0) {
            transparentCount++;
        } else {
            opaqueCount++;
        }
    }
    ImGui::Text("  Opaque: %d", opaqueCount);
    ImGui::Text("  Transparent: %d", transparentCount);
    if (!tex.pixels.empty()) {
        float transparencyPct = 100.0f * static_cast<float>(transparentCount) /
                              static_cast<float>(tex.pixels.size());
        ImGui::Text("  Transparency: %.1f%%", transparencyPct);
    }

    ImGui::EndChild();
}

}
