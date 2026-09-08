#include "views/map/MapViewPanel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"
#include "ui/WorkspaceUi.h"
#include "views/map/MapViewState.h"

namespace eld::elforge {

void MapViewPanel::render(
    CacheExplorerState& state
) const {
    if (!state.activeMap.has_value()) {
        return;
    }

    const MapViewState& map =
        *state.activeMap;

    const ImVec2 viewportPosition{
        static_cast<float>(
            state.viewportX
        ),
        static_cast<float>(
            state.viewportY
        )
    };

    const ImVec2 viewportSize{
        static_cast<float>(
            state.viewportWidth
        ),
        static_cast<float>(
            state.viewportHeight
        )
    };

    const viewport_workspace::BottomRow layout =
        viewport_workspace::bottomRow(
            viewportSize.x
        );

    ui::workspace::beginBottomHud(
        "##MapBottomHud",
        viewportPosition,
        viewportSize
    );

    const float cardY =
        ui::workspace::cardY(
            viewportPosition,
            viewportSize
        );


    // --------------------------------------------------------
    // LEFT — plane + visibility
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##MapViewCard",
        ImVec2(
            viewportPosition.x +
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
        "PLANE",
        7.0f,
        true
    );

    constexpr float PlaneWidth =
        42.0f;

    constexpr float PlaneGap =
        6.0f;

    const float planeRowWidth =
        PlaneWidth *
            static_cast<float>(
                eld::map::PlaneCount
            ) +
        PlaneGap *
            static_cast<float>(
                eld::map::PlaneCount -
                1
            );

    ImGui::SetCursorPos(
        ImVec2(
            (
                viewport_workspace::
                    LeftWidth -
                planeRowWidth
            ) *
                0.5f,
            28.0f
        )
    );

    for (
        std::size_t plane = 0;
        plane < eld::map::PlaneCount;
        ++plane
    ) {
        if (plane != 0) {
            ImGui::SameLine(
                0.0f,
                PlaneGap
            );
        }

        const std::string label =
            std::to_string(
                plane
            );

        if (
            ui::workspace::pillButton(
                (
                    "MapPlane" +
                    label
                ).c_str(),
                label.c_str(),
                state.mapPlane ==
                    plane,
                ImVec2(
                    PlaneWidth,
                    24.0f
                )
            )
        ) {
            state.mapPlane =
                plane;

            state.selectedMapTile.reset();
            state.selectedMapLocIndex.reset();

            state.mapViewportDirty =
                true;
        }
    }


    constexpr float ToggleGap =
        6.0f;

    constexpr float TerrainWidth =
        78.0f;

    constexpr float ObjectWidth =
        78.0f;

    constexpr float ResetWidth =
        66.0f;

    constexpr float ToggleRowWidth =
        TerrainWidth +
        ObjectWidth +
        ResetWidth +
        ToggleGap *
            2.0f;

    ImGui::SetCursorPos(
        ImVec2(
            (
                viewport_workspace::
                    LeftWidth -
                ToggleRowWidth
            ) *
                0.5f,
            59.0f
        )
    );

    if (
        ui::workspace::pillButton(
            "MapTerrain",
            "TERRAIN",
            state.mapShowTerrain,
            ImVec2(
                TerrainWidth,
                24.0f
            )
        )
    ) {
        state.mapShowTerrain =
            !state.mapShowTerrain;

        if (!state.mapShowTerrain) {
            state.selectedMapTile.reset();
        }

        state.mapViewportDirty =
            true;
    }

    ImGui::SameLine(
        0.0f,
        ToggleGap
    );

    if (
        ui::workspace::pillButton(
            "MapObjects",
            "OBJECTS",
            state.mapShowLocs,
            ImVec2(
                ObjectWidth,
                24.0f
            )
        )
    ) {
        state.mapShowLocs =
            !state.mapShowLocs;

        if (!state.mapShowLocs) {
            state.selectedMapLocIndex.reset();
        }

        state.mapViewportDirty =
            true;
    }

    ImGui::SameLine(
        0.0f,
        ToggleGap
    );

    if (
        ui::workspace::pillButton(
            "MapReset",
            "RESET",
            false,
            ImVec2(
                ResetWidth,
                24.0f
            )
        )
    ) {
        resetMapView(
            state.mapPlane,
            state.mapYaw,
            state.mapPitch,
            state.mapDistance
        );

        state.selectedMapTile.reset();
        state.selectedMapLocIndex.reset();

        state.mapViewportDirty =
            true;
    }

    ui::workspace::endCard();


    // --------------------------------------------------------
    // CENTER — region identity
    // --------------------------------------------------------

    if (
        layout.centerWidth >=
        150.0f
    ) {
        ui::workspace::beginCard(
            "##MapRegionCard",
            ImVec2(
                viewportPosition.x +
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
            "REGION",
            7.0f,
            true
        );

        ui::workspace::centeredText(
            "#" +
                std::to_string(
                    map.indexEntry.regionId
                ),
            29.0f
        );

        ui::workspace::centeredText(
            std::to_string(
                map.indexEntry.regionX()
            ) +
                ", " +
                std::to_string(
                    map.indexEntry.regionY()
                ),
            50.0f,
            true
        );

        ui::workspace::centeredText(
            "world " +
                std::to_string(
                    map.centerRegion.worldBaseX()
                ) +
                ", " +
                std::to_string(
                    map.centerRegion.worldBaseY()
                ),
            69.0f,
            true
        );

        ui::workspace::endCard();
    }


    // --------------------------------------------------------
    // RIGHT — scene facts
    // --------------------------------------------------------

    ui::workspace::beginCard(
        "##MapSceneCard",
        ImVec2(
            viewportPosition.x +
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
        "SCENE",
        7.0f,
        true
    );

    ui::workspace::centeredText(
        std::to_string(
            map.stats.locPlacements
        ) +
            " objects  ·  " +
            std::to_string(
                map.stats.locModelVariants
            ) +
            " variants",
        29.0f
    );

    ui::workspace::centeredText(
        std::to_string(
            map.stats.terrainTriangles
        ) +
            " terrain tris  ·  " +
            std::to_string(
                map.stats.locTriangles
            ) +
            " object tris",
        50.0f,
        true
    );

    ui::workspace::centeredText(
        std::to_string(
            static_cast<int>(
                map.stats.buildMilliseconds
            )
        ) +
            " ms build  ·  " +
            std::to_string(
                map.stats.neighborhoodRegions
            ) +
            "/9 regions",
        69.0f,
        true
    );

    ui::workspace::endCard();

    ui::workspace::endBottomHud();
}

}
