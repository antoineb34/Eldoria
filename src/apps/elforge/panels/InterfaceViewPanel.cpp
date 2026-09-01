#include "InterfaceViewPanel.h"

#include <imgui.h>

namespace eld::elforge {

void InterfaceViewPanel::render(bool hasInterface) {
    if (!hasInterface) {
        ImGui::TextDisabled(
            "Select an interface to use these controls."
        );
    }

    ImGui::BeginDisabled(!hasInterface);

    renderPresets();

    ImGui::Spacing();
    ImGui::TextUnformatted("CONTENT");
    ImGui::Separator();

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

    ImGui::Spacing();
    ImGui::TextUnformatted("LAYOUT OVERLAYS");
    ImGui::Separator();

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
