#include "TextureViewPanel.h"

#include <imgui.h>

#include "PanelUi.h"

namespace eld::elforge {

float TextureViewOptions::fixedScale() const {
    switch (zoomMode) {
        case TextureZoomMode::OneX:
            return 1.0f;
        case TextureZoomMode::TwoX:
            return 2.0f;
        case TextureZoomMode::FourX:
            return 4.0f;
        case TextureZoomMode::EightX:
            return 8.0f;
        case TextureZoomMode::Fit:
        default:
            return 1.0f;
    }
}

void TextureViewPanel::render(bool hasTexture) {
    if (!hasTexture) {
        ImGui::TextDisabled(
            "Select a texture to use these controls."
        );
    }

    ImGui::BeginDisabled(!hasTexture);

    renderPresets();

    ui::sectionHeader("ZOOM");

    static const char* ZoomModes[] = {
        "Fit",
        "1x",
        "2x",
        "4x",
        "8x"
    };

    int zoom = static_cast<int>(options_.zoomMode);

    ImGui::SetNextItemWidth(120.0f);

    if (
        ImGui::Combo(
            "##TextureZoom",
            &zoom,
            ZoomModes,
            5
        )
    ) {
        options_.zoomMode =
            static_cast<TextureZoomMode>(zoom);
    }

    ImGui::SameLine();
    ImGui::Checkbox("Nearest", &options_.nearestSampling);

    ui::sectionHeader("CHANNELS");

    ImGui::Checkbox("R", &options_.showRed);
    ImGui::SameLine();
    ImGui::Checkbox("G", &options_.showGreen);
    ImGui::SameLine();
    ImGui::Checkbox("B", &options_.showBlue);
    ImGui::SameLine();
    ImGui::Checkbox("A", &options_.showAlpha);
    ImGui::SameLine();
    ImGui::Checkbox("Alpha only", &options_.alphaOnly);

    ui::sectionHeader("OVERLAYS");

    ImGui::Checkbox(
        "Checkerboard",
        &options_.showCheckerboard
    );

    if (options_.showCheckerboard) {
        ImGui::SameLine();

        static const char* CheckerSizes[] = {
            "8 px",
            "16 px",
            "32 px"
        };

        int checkerIndex = 1;

        switch (options_.checkerSize) {
            case 8:
                checkerIndex = 0;
                break;
            case 32:
                checkerIndex = 2;
                break;
            case 16:
            default:
                checkerIndex = 1;
                break;
        }

        ImGui::SetNextItemWidth(90.0f);

        if (
            ImGui::Combo(
                "##CheckerSize",
                &checkerIndex,
                CheckerSizes,
                3
            )
        ) {
            static const int Sizes[] = {
                8,
                16,
                32
            };

            options_.checkerSize = Sizes[checkerIndex];
        }
    }

    ImGui::Checkbox("Pixel grid", &options_.showPixelGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Border", &options_.showBorder);

    ImGui::EndDisabled();
}

const TextureViewOptions& TextureViewPanel::options() const {
    return options_;
}

void TextureViewPanel::renderPresets() {
    if (ImGui::Button("Default")) {
        options_ = {};
    }

    ImGui::SameLine();

    if (ImGui::Button("Pixel inspect")) {
        options_ = {};
        options_.zoomMode = TextureZoomMode::EightX;
        options_.showPixelGrid = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Alpha")) {
        options_ = {};
        options_.alphaOnly = true;
        options_.zoomMode = TextureZoomMode::FourX;
    }
}

}
