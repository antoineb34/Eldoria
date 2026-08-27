#pragma once

#include <algorithm>

#include <imgui.h>

namespace eld::elforge {

enum class TextureZoomMode {
    Fit,
    OneX,
    TwoX,
    FourX,
    EightX
};

struct TextureViewOptions {
    bool showCheckerboard = true;
    int checkerSize = 16;

    TextureZoomMode zoomMode =
        TextureZoomMode::Fit;

    bool nearestSampling = true;

    bool showRed = true;
    bool showGreen = true;
    bool showBlue = true;
    bool showAlpha = true;
    bool alphaOnly = false;

    bool showBorder = true;
    bool showPixelGrid = false;

    float fixedScale() const {
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
};

class TextureViewPanel {
public:
    void render(
        bool hasTexture
    ) {
        if (!hasTexture) {
            ImGui::TextDisabled(
                "Select a texture to use these controls."
            );
        }

        ImGui::BeginDisabled(
            !hasTexture
        );

        renderPresets();

        ImGui::Spacing();
        ImGui::TextUnformatted("ZOOM");
        ImGui::Separator();

        static const char* ZoomModes[] = {
            "Fit",
            "1x",
            "2x",
            "4x",
            "8x"
        };

        int zoom =
            static_cast<int>(
                options_.zoomMode
            );

        ImGui::SetNextItemWidth(
            120.0f
        );

        if (
            ImGui::Combo(
                "##TextureZoom",
                &zoom,
                ZoomModes,
                5
            )
        ) {
            options_.zoomMode =
                static_cast<TextureZoomMode>(
                    zoom
                );
        }

        ImGui::SameLine();

        ImGui::Checkbox(
            "Nearest",
            &options_.nearestSampling
        );

        ImGui::Spacing();
        ImGui::TextUnformatted("CHANNELS");
        ImGui::Separator();

        ImGui::Checkbox(
            "R",
            &options_.showRed
        );

        ImGui::SameLine();

        ImGui::Checkbox(
            "G",
            &options_.showGreen
        );

        ImGui::SameLine();

        ImGui::Checkbox(
            "B",
            &options_.showBlue
        );

        ImGui::SameLine();

        ImGui::Checkbox(
            "A",
            &options_.showAlpha
        );

        ImGui::SameLine();

        ImGui::Checkbox(
            "Alpha only",
            &options_.alphaOnly
        );

        ImGui::Spacing();
        ImGui::TextUnformatted("OVERLAYS");
        ImGui::Separator();

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

            ImGui::SetNextItemWidth(
                90.0f
            );

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

                options_.checkerSize =
                    Sizes[checkerIndex];
            }
        }

        ImGui::Checkbox(
            "Pixel grid",
            &options_.showPixelGrid
        );

        ImGui::SameLine();

        ImGui::Checkbox(
            "Border",
            &options_.showBorder
        );

        ImGui::EndDisabled();
    }

    const TextureViewOptions& options() const {
        return options_;
    }

private:
    void renderPresets() {
        if (ImGui::Button("Default")) {
            options_ = {};
        }

        ImGui::SameLine();

        if (ImGui::Button("Pixel inspect")) {
            options_ = {};

            options_.zoomMode =
                TextureZoomMode::EightX;

            options_.showPixelGrid =
                true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Alpha")) {
            options_ = {};

            options_.alphaOnly =
                true;

            options_.zoomMode =
                TextureZoomMode::FourX;
        }
    }

    TextureViewOptions options_;
};

}
