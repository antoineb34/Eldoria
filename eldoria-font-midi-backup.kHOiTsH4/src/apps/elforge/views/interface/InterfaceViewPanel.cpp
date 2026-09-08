#include "views/interface/InterfaceViewPanel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "ui/PanelUi.h"
#include "ui/WorkspaceUi.h"
#include "explorer/CacheExplorerState.h"

namespace eld::elforge {

void InterfaceViewPanel::render(bool hasInterface) {
    if (!hasInterface) {
        ImGui::TextDisabled(
            "Select an interface to use these controls."
        );
    }

    ImGui::BeginDisabled(!hasInterface);

    renderPresets();

    ui::sectionHeader("CONTENT");

    ImGui::Checkbox(
        "Show hidden widgets",
        &options_.showHiddenWidgets
    );

    ImGui::Checkbox("Rectangles", &options_.showRectangles);
    ImGui::SameLine();
    ImGui::Checkbox("Text", &options_.showText);
    ImGui::SameLine();
    ImGui::Checkbox("Sprites", &options_.showSprites);

    ImGui::Checkbox("Models", &options_.showModels);
    ImGui::SameLine();
    ImGui::Checkbox("Inventories", &options_.showInventories);
    ImGui::SameLine();
    ImGui::Checkbox("Item lists", &options_.showItemLists);

    ui::sectionHeader("LAYOUT OVERLAYS");

    ImGui::Checkbox(
        "Canvas bounds",
        &options_.showCanvasBounds
    );

    ImGui::Checkbox("Grid", &options_.showGrid);

    if (options_.showGrid) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);

        static const char* GridSizes[] = {
            "4 px",
            "8 px",
            "16 px",
            "32 px"
        };

        int gridIndex = 2;

        switch (options_.gridSpacing) {
            case 4:
                gridIndex = 0;
                break;
            case 8:
                gridIndex = 1;
                break;
            case 16:
                gridIndex = 2;
                break;
            case 32:
                gridIndex = 3;
                break;
            default:
                gridIndex = 2;
                break;
        }

        if (
            ImGui::Combo(
                "##InterfaceGridSpacing",
                &gridIndex,
                GridSizes,
                4
            )
        ) {
            static const int Values[] = {
                4,
                8,
                16,
                32
            };

            options_.gridSpacing = Values[gridIndex];
        }
    }

    ImGui::Checkbox(
        "Widget bounds",
        &options_.showWidgetBounds
    );
    ImGui::SameLine();
    ImGui::Checkbox(
        "Container bounds",
        &options_.showContainerBounds
    );

    ImGui::Checkbox(
        "Clip regions",
        &options_.showClipRegions
    );
    ImGui::SameLine();
    ImGui::Checkbox(
        "Scroll extents",
        &options_.showScrollExtents
    );

    ImGui::Checkbox("Widget IDs", &options_.showWidgetIds);
    ImGui::SameLine();
    ImGui::Checkbox("Widget types", &options_.showWidgetTypes);

    ImGui::Checkbox(
        "Widget origins",
        &options_.showWidgetOrigins
    );
    ImGui::SameLine();
    ImGui::Checkbox("Parent links", &options_.showParentLinks);

    ImGui::EndDisabled();
}


void InterfaceViewPanel::renderWorkspace(
    CacheExplorerState& state,
    bool hasInterface,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    if (
        !hasInterface ||
        !state.activeInterface.has_value()
    ) {
        return;
    }


    const viewport_workspace::BottomRow layout =
        viewport_workspace::bottomRow(
            controlsSize.x
        );

    ui::workspace::beginDockedHud(
        "##InterfaceBottomHud",
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
        "##InterfacePresetCard",
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

    constexpr float PresetGap =
        6.0f;

    constexpr float CleanWidth =
        64.0f;

    constexpr float DebugWidth =
        76.0f;

    constexpr float ModelsWidth =
        84.0f;

    constexpr float PresetWidth =
        CleanWidth +
        DebugWidth +
        ModelsWidth +
        PresetGap *
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
            "InterfaceClean",
            "CLEAN",
            false,
            ImVec2(
                CleanWidth,
                24.0f
            )
        )
    ) {
        options_ = {};
    }

    ImGui::SameLine(
        0.0f,
        PresetGap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceDebug",
            "DEBUG",
            false,
            ImVec2(
                DebugWidth,
                24.0f
            )
        )
    ) {
        options_ = {};

        options_.showGrid = true;
        options_.showWidgetBounds = true;
        options_.showContainerBounds = true;
        options_.showClipRegions = true;
        options_.showWidgetIds = true;
        options_.showWidgetOrigins = true;
        options_.showScrollExtents = true;
    }

    ImGui::SameLine(
        0.0f,
        PresetGap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceModels",
            "MODELS",
            false,
            ImVec2(
                ModelsWidth,
                24.0f
            )
        )
    ) {
        options_ = {};

        options_.showRectangles = false;
        options_.showText = false;
        options_.showSprites = false;
        options_.showInventories = false;
        options_.showItemLists = false;
    }

    ui::workspace::endCard();


    // --------------------------------------------------------
    // CENTER — content channels
    // --------------------------------------------------------

    if (
        layout.centerWidth >=
        150.0f
    ) {
        ui::workspace::beginCard(
            "##InterfaceContentCard",
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
            "CONTENT",
            7.0f,
            true
        );

        constexpr float Gap =
            5.0f;

        constexpr float Small =
            58.0f;

        constexpr float Medium =
            70.0f;

        const float firstWidth =
            Small *
                3.0f +
            Gap *
                2.0f;

        ImGui::SetCursorPos(
            ImVec2(
                std::max(
                    (
                        layout.centerWidth -
                        firstWidth
                    ) *
                        0.5f,
                    6.0f
                ),
                28.0f
            )
        );

        if (
            ui::workspace::pillButton(
                "InterfaceRectangles",
                "RECT",
                options_.showRectangles,
                ImVec2(
                    Small,
                    24.0f
                )
            )
        ) {
            options_.showRectangles =
                !options_.showRectangles;
        }

        ImGui::SameLine(
            0.0f,
            Gap
        );

        if (
            ui::workspace::pillButton(
                "InterfaceText",
                "TEXT",
                options_.showText,
                ImVec2(
                    Small,
                    24.0f
                )
            )
        ) {
            options_.showText =
                !options_.showText;
        }

        ImGui::SameLine(
            0.0f,
            Gap
        );

        if (
            ui::workspace::pillButton(
                "InterfaceSprites",
                "SPR",
                options_.showSprites,
                ImVec2(
                    Small,
                    24.0f
                )
            )
        ) {
            options_.showSprites =
                !options_.showSprites;
        }


        const float secondWidth =
            Medium *
                3.0f +
            Gap *
                2.0f;

        ImGui::SetCursorPos(
            ImVec2(
                std::max(
                    (
                        layout.centerWidth -
                        secondWidth
                    ) *
                        0.5f,
                    6.0f
                ),
                59.0f
            )
        );

        if (
            ui::workspace::pillButton(
                "InterfaceModelsToggle",
                "MODEL",
                options_.showModels,
                ImVec2(
                    Medium,
                    24.0f
                )
            )
        ) {
            options_.showModels =
                !options_.showModels;
        }

        ImGui::SameLine(
            0.0f,
            Gap
        );

        if (
            ui::workspace::pillButton(
                "InterfaceInventories",
                "INV",
                options_.showInventories,
                ImVec2(
                    Medium,
                    24.0f
                )
            )
        ) {
            options_.showInventories =
                !options_.showInventories;
        }

        ImGui::SameLine(
            0.0f,
            Gap
        );

        if (
            ui::workspace::pillButton(
                "InterfaceLists",
                "LIST",
                options_.showItemLists,
                ImVec2(
                    Medium,
                    24.0f
                )
            )
        ) {
            options_.showItemLists =
                !options_.showItemLists;
        }

        ui::workspace::endCard();
    }


    // --------------------------------------------------------
    // RIGHT — layout/debug overlays
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##InterfaceLayoutCard",
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
        "LAYOUT",
        7.0f,
        true
    );

    constexpr float Gap =
        6.0f;

    constexpr float ToggleWidth =
        72.0f;

    constexpr float WideWidth =
        86.0f;

    constexpr float MoreWidth =
        68.0f;

    const float firstWidth =
        WideWidth +
        ToggleWidth *
            3.0f +
        Gap *
            3.0f;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    layout.rightWidth -
                    firstWidth
                ) *
                    0.5f,
                8.0f
            ),
            28.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "InterfaceHidden",
            "HIDDEN",
            options_.showHiddenWidgets,
            ImVec2(
                WideWidth,
                24.0f
            )
        )
    ) {
        options_.showHiddenWidgets =
            !options_.showHiddenWidgets;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceGrid",
            "GRID",
            options_.showGrid,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showGrid =
            !options_.showGrid;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceBounds",
            "BOUNDS",
            options_.showWidgetBounds,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showWidgetBounds =
            !options_.showWidgetBounds;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceClip",
            "CLIP",
            options_.showClipRegions,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showClipRegions =
            !options_.showClipRegions;
    }


    const float secondWidth =
        ToggleWidth *
            3.0f +
        MoreWidth +
        Gap *
            3.0f;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    layout.rightWidth -
                    secondWidth
                ) *
                    0.5f,
                8.0f
            ),
            59.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "InterfaceIds",
            "IDS",
            options_.showWidgetIds,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showWidgetIds =
            !options_.showWidgetIds;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceOrigins",
            "ORIGIN",
            options_.showWidgetOrigins,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showWidgetOrigins =
            !options_.showWidgetOrigins;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceParent",
            "PARENT",
            options_.showParentLinks,
            ImVec2(
                ToggleWidth,
                24.0f
            )
        )
    ) {
        options_.showParentLinks =
            !options_.showParentLinks;
    }

    ImGui::SameLine(
        0.0f,
        Gap
    );

    if (
        ui::workspace::pillButton(
            "InterfaceMore",
            "MORE",
            false,
            ImVec2(
                MoreWidth,
                24.0f
            )
        )
    ) {
        ImGui::OpenPopup(
            "##InterfaceWorkspaceMore"
        );
    }

    if (
        ImGui::BeginPopup(
            "##InterfaceWorkspaceMore"
        )
    ) {
        ImGui::TextUnformatted(
            "INTERFACE DISPLAY"
        );

        ImGui::Separator();

        ImGui::Checkbox(
            "Container bounds",
            &options_.showContainerBounds
        );

        ImGui::Checkbox(
            "Scroll extents",
            &options_.showScrollExtents
        );

        ImGui::Checkbox(
            "Widget types",
            &options_.showWidgetTypes
        );

        ImGui::Checkbox(
            "Canvas bounds",
            &options_.showCanvasBounds
        );

        if (options_.showGrid) {
            ImGui::Separator();

            ImGui::TextDisabled(
                "Grid spacing"
            );

            static const int Values[] = {
                4,
                8,
                16,
                32
            };

            for (
                int value :
                Values
            ) {
                ImGui::SameLine();

                const std::string label =
                    std::to_string(
                        value
                    );

                if (
                    ui::workspace::pillButton(
                        (
                            "InterfaceGrid" +
                            label
                        ).c_str(),
                        label.c_str(),
                        options_.gridSpacing ==
                            value,
                        ImVec2(
                            42.0f,
                            24.0f
                        )
                    )
                ) {
                    options_.gridSpacing =
                        value;
                }
            }
        }

        ImGui::EndPopup();
    }

    ui::workspace::endCard();

    ui::workspace::endDockedHud();
}

const InterfaceViewOptions& InterfaceViewPanel::options() const {
    return options_;
}

void InterfaceViewPanel::renderPresets() {
    if (ImGui::Button("Clean")) {
        options_ = {};
    }

    ImGui::SameLine();

    if (ImGui::Button("Layout debug")) {
        options_ = {};
        options_.showGrid = true;
        options_.showWidgetBounds = true;
        options_.showContainerBounds = true;
        options_.showClipRegions = true;
        options_.showWidgetIds = true;
        options_.showWidgetOrigins = true;
        options_.showScrollExtents = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Models only")) {
        options_ = {};
        options_.showRectangles = false;
        options_.showText = false;
        options_.showSprites = false;
        options_.showInventories = false;
        options_.showItemLists = false;
    }
}

}
