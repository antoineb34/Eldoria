#include "TexturePreviewPanel.h"

#include <imgui.h>
#include <algorithm>

namespace eldoria::apps::elforge {

void TexturePreviewPanel::render(
    CacheState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "TexturePreviewPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("TEXTURE PREVIEW");
    ImGui::Separator();

    if (!state.selectedTexture.has_value()) {
        ImGui::TextUnformatted("No texture loaded.");
        ImGui::EndChild();
        return;
    }

    const auto& tex = *state.selectedTexture;
    const int texW = tex.metadata.width;
    const int texH = tex.metadata.height;

    if (texW <= 0 || texH <= 0) {
        ImGui::TextUnformatted("Texture has invalid dimensions.");
        ImGui::EndChild();
        return;
    }

    ImGui::Text(
        "Size: %d x %d  Canvas: %d x %d",
        texW, texH,
        tex.metadata.canvasWidth,
        tex.metadata.canvasHeight
    );

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float availW = avail.x;
    float availH = avail.y;
    if (availW < 1.0f) availW = 1.0f;
    if (availH < 1.0f) availH = 1.0f;

    float scaleX = availW / static_cast<float>(texW);
    float scaleY = availH / static_cast<float>(texH);
    float scale = std::min(scaleX, scaleY);
    scale = std::min(scale, 8.0f);

    float displayW = static_cast<float>(texW) * scale;
    float displayH = static_cast<float>(texH) * scale;

    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        cursorPos,
        ImVec2(cursorPos.x + displayW, cursorPos.y + displayH),
        IM_COL32(20, 20, 25, 255)
    );

    if (!tex.pixels.empty() && texW > 0 && texH > 0) {
        int stepX = std::max(1, texW / 128);
        int stepY = std::max(1, texH / 128);
        for (int y = 0; y < texH; y += stepY) {
            for (int x = 0; x < texW; x += stepX) {
                int idx = y * texW + x;
                if (idx >= static_cast<int>(tex.pixels.size())) break;

                const auto& px = tex.pixels[idx];
                if (px.a == 0) continue;

                float pxX = cursorPos.x + (static_cast<float>(x) / texW) * displayW;
                float pxY = cursorPos.y + (static_cast<float>(y) / texH) * displayH;
                float pxSizeX = std::max(1.0f, (static_cast<float>(stepX) / texW) * displayW);
                float pxSizeY = std::max(1.0f, (static_cast<float>(stepY) / texH) * displayH);

                drawList->AddRectFilled(
                    ImVec2(pxX, pxY),
                    ImVec2(pxX + pxSizeX, pxY + pxSizeY),
                    IM_COL32(px.r, px.g, px.b, px.a)
                );
            }
        }
    }

    ImGui::Dummy(ImVec2(displayW, displayH));
    ImGui::EndChild();
}

}
