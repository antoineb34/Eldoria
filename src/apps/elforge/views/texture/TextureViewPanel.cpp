#include "views/texture/TextureViewPanel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "ui/PanelUi.h"
#include "ui/WorkspaceUi.h"
#include "explorer/CacheExplorerState.h"

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


void TextureViewPanel::renderWorkspace(
    CacheExplorerState& state,
    bool hasTexture,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    if (
        !hasTexture ||
        !state.activeTexture.has_value()
    ) {
        return;
    }


    const viewport_workspace::BottomRow layout =
        viewport_workspace::bottomRow(
            controlsSize.x
        );

    ui::workspace::beginDockedHud(
        "##TextureBottomHud",
        controlsPosition,
        controlsSize
    );

    const float cardY =
        ui::workspace::dockedCardY(
            controlsPosition,
            controlsSize
        );


    // --------------------------------------------------------
    // LEFT — presets
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##TexturePresetCard",
        ImVec2(
            controlsPosition.x +
                layout.leftX,
            cardY
        ),
        ImVec2(
            viewport_workspace::
                LeftWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "PRESET",
        7.0f,
        true
    );

    constexpr float Gap =
        6.0f;

    constexpr float DefaultWidth =
        76.0f;

    constexpr float PixelWidth =
        76.0f;

    constexpr float AlphaWidth =
        68.0f;

    constexpr float PresetWidth =
        DefaultWidth +
        PixelWidth +
        AlphaWidth +
        Gap *
            2.0f;

    ImGui::SetCursorPos(
        ImVec2(
            (
                viewport_workspace::
                    LeftWidth -
                PresetWidth
            ) *
                0.5f,
            37.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "TextureDefault",
            "DEFAULT",
            false,
            ImVec2(
                DefaultWidth,
                24.0f
            )
        )
    ) {
        options_ = {};
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "TexturePixel",
            "PIXEL",
            false,
            ImVec2(
                PixelWidth,
                24.0f
            )
        )
    ) {
        options_ = {};

        options_.zoomMode =
            TextureZoomMode::EightX;

        options_.showPixelGrid =
            true;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "TextureAlphaPreset",
            "ALPHA",
            false,
            ImVec2(
                AlphaWidth,
                24.0f
            )
        )
    ) {
        options_ = {};

        options_.alphaOnly =
            true;

        options_.zoomMode =
            TextureZoomMode::FourX;
    }

    ui::workspace::endCard();


    // --------------------------------------------------------
    // CENTER — zoom
    // --------------------------------------------------------

    if (
        layout.centerWidth >=
        150.0f
    ) {
        ui::workspace::beginCard(
            "##TextureZoomCard",
            ImVec2(
                controlsPosition.x +
                    layout.centerX,
                cardY
            ),
            ImVec2(
                layout.centerWidth,
                viewport_workspace::
                    CardHeight
            )
        );

        ui::workspace::centeredText(
            "ZOOM",
            7.0f,
            true
        );

        static const char* ZoomLabels[] = {
            "FIT",
            "1X",
            "2X",
            "4X",
            "8X"
        };

        constexpr int ZoomCount =
            5;

        int zoomIndex =
            static_cast<int>(
                options_.zoomMode
            );

        constexpr float ArrowWidth =
            26.0f;

        constexpr float LabelWidth =
            86.0f;

        constexpr float ZoomGap =
            6.0f;

        constexpr float ZoomRowWidth =
            ArrowWidth +
            LabelWidth +
            ArrowWidth +
            ZoomGap *
                2.0f;

        ImGui::SetCursorPos(
            ImVec2(
                std::max(
                    (
                        layout.centerWidth -
                        ZoomRowWidth
                    ) *
                        0.5f,
                    6.0f
                ),
                28.0f
            )
        );

        if (
            ui::workspace::navigationButton(
                "##TextureZoomPrevious",
                ui::Icon::ChevronLeft,
                "Previous zoom"
            )
        ) {
            zoomIndex =
                (
                    zoomIndex -
                    1 +
                    ZoomCount
                ) %
                ZoomCount;

            options_.zoomMode =
                static_cast<TextureZoomMode>(
                    zoomIndex
                );
        }

        ImGui::SameLine(
            0.0f,
            ZoomGap
        );

        if (
            ui::workspace::pillButton(
                "TextureZoomValue",
                ZoomLabels[
                    zoomIndex
                ],
                false,
                ImVec2(
                    LabelWidth,
                    24.0f
                )
            )
        ) {
            options_.zoomMode =
                TextureZoomMode::Fit;
        }

        ImGui::SameLine(
            0.0f,
            ZoomGap
        );

        if (
            ui::workspace::navigationButton(
                "##TextureZoomNext",
                ui::Icon::ChevronRight,
                "Next zoom"
            )
        ) {
            zoomIndex =
                (
                    zoomIndex +
                    1
                ) %
                ZoomCount;

            options_.zoomMode =
                static_cast<TextureZoomMode>(
                    zoomIndex
                );
        }


        ImGui::SetCursorPos(
            ImVec2(
                std::max(
                    (
                        layout.centerWidth -
                        92.0f
                    ) *
                        0.5f,
                    6.0f
                ),
                59.0f
            )
        );

        if (
            ui::workspace::pillButton(
                "TextureNearest",
                "NEAREST",
                options_.nearestSampling,
                ImVec2(
                    92.0f,
                    24.0f
                )
            )
        ) {
            options_.nearestSampling =
                !options_.nearestSampling;
        }

        ui::workspace::endCard();
    }


    // --------------------------------------------------------
    // RIGHT — channels / overlays
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##TextureDisplayCard",
        ImVec2(
            controlsPosition.x +
                layout.rightX,
            cardY
        ),
        ImVec2(
            layout.rightWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "DISPLAY",
        7.0f,
        true
    );

    constexpr float ChannelGap =
        6.0f;

    constexpr float ChannelWidth =
        44.0f;

    constexpr float AlphaOnlyWidth =
        86.0f;

    constexpr float FirstWidth =
        ChannelWidth *
            4.0f +
        AlphaOnlyWidth +
        ChannelGap *
            4.0f;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    layout.rightWidth -
                    FirstWidth
                ) *
                    0.5f,
                8.0f
            ),
            28.0f
        )
    );

    const auto channelToggle =
        [](
            const char* id,
            const char* label,
            bool& value
        ) {
            if (
                ui::workspace::pillButton(
                    id,
                    label,
                    value,
                    ImVec2(
                        ChannelWidth,
                        24.0f
                    )
                )
            ) {
                value =
                    !value;
            }
        };

    channelToggle(
        "TextureR",
        "R",
        options_.showRed
    );

    ImGui::SameLine(
        0.0f,
        ChannelGap
    );

    channelToggle(
        "TextureG",
        "G",
        options_.showGreen
    );

    ImGui::SameLine(
        0.0f,
        ChannelGap
    );

    channelToggle(
        "TextureB",
        "B",
        options_.showBlue
    );

    ImGui::SameLine(
        0.0f,
        ChannelGap
    );

    channelToggle(
        "TextureA",
        "A",
        options_.showAlpha
    );

    ImGui::SameLine(
        0.0f,
        ChannelGap
    );

    if (
        ui::workspace::pillButton(
            "TextureAlphaOnly",
            "ALPHA",
            options_.alphaOnly,
            ImVec2(
                AlphaOnlyWidth,
                24.0f
            )
        )
    ) {
        options_.alphaOnly =
            !options_.alphaOnly;
    }


    constexpr float OverlayGap =
        6.0f;

    constexpr float CheckerWidth =
        92.0f;

    constexpr float GridWidth =
        68.0f;

    constexpr float BorderWidth =
        76.0f;

    constexpr float MoreWidth =
        68.0f;

    constexpr float SecondWidth =
        CheckerWidth +
        GridWidth +
        BorderWidth +
        MoreWidth +
        OverlayGap *
            3.0f;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    layout.rightWidth -
                    SecondWidth
                ) *
                    0.5f,
                8.0f
            ),
            59.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "TextureChecker",
            "CHECKER",
            options_.showCheckerboard,
            ImVec2(
                CheckerWidth,
                24.0f
            )
        )
    ) {
        options_.showCheckerboard =
            !options_.showCheckerboard;
    }

    ImGui::SameLine(
        0.0f,
        OverlayGap
    );

    if (
        ui::workspace::pillButton(
            "TextureGrid",
            "GRID",
            options_.showPixelGrid,
            ImVec2(
                GridWidth,
                24.0f
            )
        )
    ) {
        options_.showPixelGrid =
            !options_.showPixelGrid;
    }

    ImGui::SameLine(
        0.0f,
        OverlayGap
    );

    if (
        ui::workspace::pillButton(
            "TextureBorder",
            "BORDER",
            options_.showBorder,
            ImVec2(
                BorderWidth,
                24.0f
            )
        )
    ) {
        options_.showBorder =
            !options_.showBorder;
    }

    ImGui::SameLine(
        0.0f,
        OverlayGap
    );

    if (
        ui::workspace::pillButton(
            "TextureMore",
            "MORE",
            false,
            ImVec2(
                MoreWidth,
                24.0f
            )
        )
    ) {
        ImGui::OpenPopup(
            "##TextureWorkspaceMore"
        );
    }

    if (
        ImGui::BeginPopup(
            "##TextureWorkspaceMore"
        )
    ) {
        ImGui::TextUnformatted(
            "TEXTURE DISPLAY"
        );

        ImGui::Separator();

        ImGui::TextDisabled(
            "Checker size"
        );

        static const int Sizes[] = {
            8,
            16,
            32
        };

        for (
            int size :
            Sizes
        ) {
            ImGui::SameLine();

            const std::string label =
                std::to_string(
                    size
                );

            if (
                ui::workspace::pillButton(
                    (
                        "TextureChecker" +
                        label
                    ).c_str(),
                    label.c_str(),
                    options_.checkerSize ==
                        size,
                    ImVec2(
                        46.0f,
                        24.0f
                    )
                )
            ) {
                options_.checkerSize =
                    size;
            }
        }

        ImGui::EndPopup();
    }

    ui::workspace::endCard();

    ui::workspace::endDockedHud();
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
