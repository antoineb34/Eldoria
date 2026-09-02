#include "views/model/ModelViewPanel.h"

#include <algorithm>
#include <string>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"

#include "ui/ElForgeTheme.h"
#include "ui/PanelUi.h"
#include "ui/WorkspaceUi.h"

#include "viewport/ViewportWorkspaceLayout.h"


namespace eld::elforge {

namespace {


const char* presetLabel(
    ModelViewPreset preset
) {
    switch (preset) {
        case ModelViewPreset::Iso:
            return "ISO";

        case ModelViewPreset::Front:
            return "FRONT";

        case ModelViewPreset::Back:
            return "BACK";

        case ModelViewPreset::Left:
            return "LEFT";

        case ModelViewPreset::Right:
            return "RIGHT";

        case ModelViewPreset::Top:
            return "TOP";

        case ModelViewPreset::Bottom:
            return "BOTTOM";
    }

    return "VIEW";
}


const char* backgroundLabel(
    ModelBackground background
) {
    switch (background) {
        case ModelBackground::Dark:
            return "DARK";

        case ModelBackground::Light:
            return "LIGHT";

        case ModelBackground::Neutral:
        default:
            return "NEUTRAL";
    }
}


void centeredText(
    const std::string& text,
    float y,
    bool disabled = false
) {
    ui::workspace::centeredText(
        text,
        y,
        disabled
    );
}


bool pillButton(
    const char* id,
    const char* label,
    bool active,
    const ImVec2& size
) {
    return
        ui::workspace::pillButton(
            id,
            label,
            active,
            size
        );
}


bool quietArrow(
    const char* id,
    ImGuiDir direction
) {
    return
        ui::workspace::navigationButton(
            id,
            direction == ImGuiDir_Left
                ? ui::Icon::ChevronLeft
                : ui::Icon::ChevronRight,
            direction == ImGuiDir_Left
                ? "Previous"
                : "Next"
        );
}


void beginWorkspaceCard(
    const char* id,
    const ImVec2& position,
    const ImVec2& size
) {
    ui::workspace::beginCard(
        id,
        position,
        size
    );
}


void endWorkspaceCard() {
    ui::workspace::endCard();
}


}


std::array<std::uint8_t, 4>
ModelViewOptions::backgroundColor() const {
    switch (background) {
        case ModelBackground::Dark:
            return {
                28,
                30,
                34,
                255
            };

        case ModelBackground::Light:
            return {
                185,
                185,
                185,
                255
            };

        case ModelBackground::Neutral:
        default:
            return {
                68,
                88,
                68,
                255
            };
    }
}


void ModelViewPanel::update(
    CacheExplorerState& state,
    bool hasModel
) {
    if (
        !hasModel ||
        !options_.autoRotate
    ) {
        return;
    }

    state.modelTransform.rotation.y +=
        options_.autoRotateSpeed *
        std::max(
            ImGui::GetIO().DeltaTime,
            0.0f
        );
}


void ModelViewPanel::render(
    CacheExplorerState& state,
    bool hasModel
) {
    if (!hasModel) {
        ImGui::TextDisabled(
            "Select a model to use these controls."
        );
    }

    ImGui::BeginDisabled(
        !hasModel
    );


    renderViewPresets(
        state
    );


    ui::sectionHeader(
        "TRANSFORM"
    );

    renderTransformControls(
        state
    );


    ui::sectionHeader(
        "DISPLAY"
    );

    ImGui::Checkbox(
        "Solid",
        &options_.showSolid
    );

    ImGui::SameLine();

    ImGui::Checkbox(
        "Wireframe",
        &options_.showWireframe
    );

    ImGui::SameLine();

    ImGui::Checkbox(
        "Vertices",
        &options_.showVertices
    );


    ImGui::Checkbox(
        "Bounds",
        &options_.showBounds
    );

    ImGui::SameLine();

    ImGui::Checkbox(
        "Axes",
        &options_.showAxes
    );


    ui::sectionHeader(
        "MOTION"
    );

    ImGui::Checkbox(
        "Auto rotate",
        &options_.autoRotate
    );

    if (options_.autoRotate) {
        ImGui::SameLine();

        ImGui::SetNextItemWidth(
            150.0f
        );

        ImGui::SliderFloat(
            "##ModelAutoRotateSpeed",
            &options_.autoRotateSpeed,
            -3.0f,
            3.0f,
            "%.2f rad/s"
        );
    }


    ui::sectionHeader(
        "BACKGROUND"
    );

    static const char* Backgrounds[] = {
        "Neutral",
        "Dark",
        "Light"
    };

    int background =
        static_cast<int>(
            options_.background
        );

    ImGui::SetNextItemWidth(
        140.0f
    );

    if (
        ImGui::Combo(
            "##ModelBackground",
            &background,
            Backgrounds,
            3
        )
    ) {
        options_.background =
            static_cast<ModelBackground>(
                background
            );
    }


    ImGui::EndDisabled();
}


void ModelViewPanel::renderWorkspace(
    CacheExplorerState& state,
    bool hasModel
) {
    if (
        !hasModel ||
        !state.activeModel.has_value()
    ) {
        return;
    }

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
        "##ModelBottomHud",
        viewportPosition,
        viewportSize
    );

    const float cardY =
        ui::workspace::cardY(
            viewportPosition,
            viewportSize
        );


    // --------------------------------------------------------
    // LEFT — view / motion
    // --------------------------------------------------------

    beginWorkspaceCard(
        "##ModelViewCard",
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

    renderWorkspaceViewCard(
        state,
        viewport_workspace::
            LeftWidth
    );

    endWorkspaceCard();


    // --------------------------------------------------------
    // CENTER — model facts
    // --------------------------------------------------------

    if (
        layout.centerWidth >=
        150.0f
    ) {
        beginWorkspaceCard(
            "##ModelInfoCard",
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

        renderWorkspaceModelCard(
            state,
            layout.centerWidth
        );

        endWorkspaceCard();
    }


    // --------------------------------------------------------
    // RIGHT — display
    // --------------------------------------------------------

    beginWorkspaceCard(
        "##ModelDisplayCard",
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

    renderWorkspaceDisplayCard(
        layout.rightWidth
    );

    endWorkspaceCard();

    ui::workspace::endBottomHud();
}



const ModelViewOptions&
ModelViewPanel::options() const {
    return options_;
}


void ModelViewPanel::setRotation(
    CacheExplorerState& state,
    float x,
    float y,
    float z
) {
    state.modelTransform.rotation = {
        x,
        y,
        z
    };
}


float ModelViewPanel::radiansToDegrees(
    float radians
) {
    return
        radians *
        180.0f /
        Pi;
}


float ModelViewPanel::degreesToRadians(
    float degrees
) {
    return
        degrees *
        Pi /
        180.0f;
}


void ModelViewPanel::applyViewPreset(
    CacheExplorerState& state
) {
    switch (options_.viewPreset) {
        case ModelViewPreset::Iso:
            setRotation(
                state,
                -0.55f,
                0.78f
            );
            break;

        case ModelViewPreset::Front:
            setRotation(
                state,
                0.0f,
                0.0f
            );
            break;

        case ModelViewPreset::Back:
            setRotation(
                state,
                0.0f,
                Pi
            );
            break;

        case ModelViewPreset::Left:
            setRotation(
                state,
                0.0f,
                -Pi *
                    0.5f
            );
            break;

        case ModelViewPreset::Right:
            setRotation(
                state,
                0.0f,
                Pi *
                    0.5f
            );
            break;

        case ModelViewPreset::Top:
            setRotation(
                state,
                -Pi *
                    0.5f,
                0.0f
            );
            break;

        case ModelViewPreset::Bottom:
            setRotation(
                state,
                Pi *
                    0.5f,
                0.0f
            );
            break;
    }
}


void ModelViewPanel::cycleViewPreset(
    CacheExplorerState& state,
    int direction
) {
    constexpr int PresetCount =
        7;

    int index =
        static_cast<int>(
            options_.viewPreset
        );

    index =
        (
            index +
            direction +
            PresetCount
        ) %
        PresetCount;

    options_.viewPreset =
        static_cast<ModelViewPreset>(
            index
        );

    options_.autoRotate =
        false;

    applyViewPreset(
        state
    );
}


void ModelViewPanel::cycleBackground(
    int direction
) {
    constexpr int BackgroundCount =
        3;

    int index =
        static_cast<int>(
            options_.background
        );

    index =
        (
            index +
            direction +
            BackgroundCount
        ) %
        BackgroundCount;

    options_.background =
        static_cast<ModelBackground>(
            index
        );
}


void ModelViewPanel::renderWorkspaceViewCard(
    CacheExplorerState& state,
    float width
) {
    centeredText(
        "VIEW",
        7.0f,
        true
    );


    // Preset carousel.
    constexpr float ArrowWidth =
        24.0f;

    constexpr float LabelWidth =
        96.0f;

    constexpr float Gap =
        6.0f;

    constexpr float PresetRowWidth =
        ArrowWidth +
        Gap +
        LabelWidth +
        Gap +
        ArrowWidth;


    ImGui::SetCursorPos(
        ImVec2(
            (
                width -
                PresetRowWidth
            ) *
                0.5f,
            28.0f
        )
    );


    if (
        quietArrow(
            "##ModelViewPrevious",
            ImGuiDir_Left
        )
    ) {
        cycleViewPreset(
            state,
            -1
        );
    }


    ImGui::SameLine(
        0.0f,
        Gap
    );


    if (
        pillButton(
            "ModelViewPreset",
            presetLabel(
                options_.viewPreset
            ),
            false,
            ImVec2(
                LabelWidth,
                24.0f
            )
        )
    ) {
        options_.autoRotate =
            false;

        applyViewPreset(
            state
        );
    }


    ImGui::SameLine(
        0.0f,
        Gap
    );


    if (
        quietArrow(
            "##ModelViewNext",
            ImGuiDir_Right
        )
    ) {
        cycleViewPreset(
            state,
            1
        );
    }


    // Auto rotation row.
    constexpr float AutoWidth =
        58.0f;

    constexpr float SpeedWidth =
        72.0f;

    constexpr float MotionGap =
        8.0f;

    constexpr float MotionWidth =
        AutoWidth +
        MotionGap +
        SpeedWidth;


    ImGui::SetCursorPos(
        ImVec2(
            (
                width -
                MotionWidth
            ) *
                0.5f,
            59.0f
        )
    );


    if (
        pillButton(
            "ModelAutoRotate",
            "AUTO",
            options_.autoRotate,
            ImVec2(
                AutoWidth,
                24.0f
            )
        )
    ) {
        options_.autoRotate =
            !options_.autoRotate;
    }


    ImGui::SameLine(
        0.0f,
        MotionGap
    );


    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        12.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameBorderSize,
        0.0f
    );

    const auto& palette =
        ui::themePalette();

    ImGui::PushStyleColor(
        ImGuiCol_FrameBg,
        ImVec4(
            palette.text.x,
            palette.text.y,
            palette.text.z,
            0.055f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgHovered,
        ImVec4(
            palette.text.x,
            palette.text.y,
            palette.text.z,
            0.10f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgActive,
        ImVec4(
            palette.primary.x,
            palette.primary.y,
            palette.primary.z,
            0.17f
        )
    );

    ImGui::SetNextItemWidth(
        SpeedWidth
    );

    ImGui::DragFloat(
        "##ModelWorkspaceAutoRotateSpeed",
        &options_.autoRotateSpeed,
        0.05f,
        -3.0f,
        3.0f,
        "%.2f"
    );

    if (
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "Auto-rotate speed (rad/s)"
        );
    }


    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}


void ModelViewPanel::renderWorkspaceModelCard(
    const CacheExplorerState& state,
    float width
) {
    if (!state.activeModel.has_value()) {
        return;
    }


    const eld::model::Model& model =
        *state.activeModel;


    centeredText(
        "MODEL",
        7.0f,
        true
    );


    centeredText(
        "#" +
            std::to_string(
                model.id
            ),
        29.0f,
        false
    );


    const std::string geometryFull =
        std::to_string(
            model.mesh.vertices.size()
        ) +
        " vertices  ·  " +
        std::to_string(
            model.mesh.faces.size()
        ) +
        " faces";

    const std::string geometryCompact =
        std::to_string(
            model.mesh.vertices.size()
        ) +
        "v  ·  " +
        std::to_string(
            model.mesh.faces.size()
        ) +
        "f";


    const std::string& geometry =
        ImGui::CalcTextSize(
            geometryFull.c_str()
        ).x <=
            width -
                18.0f
            ? geometryFull
            : geometryCompact;


    centeredText(
        geometry,
        51.0f,
        true
    );


    const std::string textureText =
        std::to_string(
            model.mesh.
                textureMappings.size()
        ) +
        (
            model.mesh.
                textureMappings.size() ==
                    1
                ? " texture map"
                : " texture maps"
        );


    centeredText(
        textureText,
        70.0f,
        true
    );
}


void ModelViewPanel::renderWorkspaceDisplayCard(
    float width
) {
    centeredText(
        "DISPLAY",
        7.0f,
        true
    );


    // --------------------------------------------------------
    // Primary geometry display.
    // --------------------------------------------------------

    constexpr float SolidWidth =
        68.0f;

    constexpr float WireWidth =
        68.0f;

    constexpr float VerticesWidth =
        84.0f;

    constexpr float Gap =
        6.0f;

    constexpr float FirstRowWidth =
        SolidWidth +
        WireWidth +
        VerticesWidth +
        Gap *
            2.0f;


    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    width -
                    FirstRowWidth
                ) *
                    0.5f,
                8.0f
            ),
            28.0f
        )
    );


    if (
        pillButton(
            "ModelSolid",
            "SOLID",
            options_.showSolid,
            ImVec2(
                SolidWidth,
                24.0f
            )
        )
    ) {
        options_.showSolid =
            !options_.showSolid;
    }


    ImGui::SameLine(
        0.0f,
        Gap
    );


    if (
        pillButton(
            "ModelWire",
            "WIRE",
            options_.showWireframe,
            ImVec2(
                WireWidth,
                24.0f
            )
        )
    ) {
        options_.showWireframe =
            !options_.showWireframe;
    }


    ImGui::SameLine(
        0.0f,
        Gap
    );


    if (
        pillButton(
            "ModelVertices",
            "VERTS",
            options_.showVertices,
            ImVec2(
                VerticesWidth,
                24.0f
            )
        )
    ) {
        options_.showVertices =
            !options_.showVertices;
    }


    // --------------------------------------------------------
    // Diagnostic overlays + background carousel.
    // --------------------------------------------------------

    constexpr float BoundsWidth =
        76.0f;

    constexpr float AxesWidth =
        58.0f;

    constexpr float BackgroundWidth =
        88.0f;

    constexpr float SecondGap =
        6.0f;

    constexpr float BackgroundGap =
        4.0f;

    constexpr float BackgroundArrowWidth =
        24.0f;

    constexpr float SecondRowWidth =
        BoundsWidth +
        SecondGap +
        AxesWidth +
        12.0f +
        BackgroundArrowWidth +
        BackgroundGap +
        BackgroundWidth +
        BackgroundGap +
        BackgroundArrowWidth;


    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    width -
                    SecondRowWidth
                ) *
                    0.5f,
                8.0f
            ),
            59.0f
        )
    );


    if (
        pillButton(
            "ModelBounds",
            "BOUNDS",
            options_.showBounds,
            ImVec2(
                BoundsWidth,
                24.0f
            )
        )
    ) {
        options_.showBounds =
            !options_.showBounds;
    }


    ImGui::SameLine(
        0.0f,
        SecondGap
    );


    if (
        pillButton(
            "ModelAxes",
            "AXES",
            options_.showAxes,
            ImVec2(
                AxesWidth,
                24.0f
            )
        )
    ) {
        options_.showAxes =
            !options_.showAxes;
    }


    ImGui::SameLine(
        0.0f,
        12.0f
    );


    if (
        quietArrow(
            "##ModelBackgroundPrevious",
            ImGuiDir_Left
        )
    ) {
        cycleBackground(
            -1
        );
    }


    ImGui::SameLine(
        0.0f,
        BackgroundGap
    );


    if (
        pillButton(
            "ModelBackground",
            backgroundLabel(
                options_.background
            ),
            false,
            ImVec2(
                BackgroundWidth,
                24.0f
            )
        )
    ) {
        cycleBackground(
            1
        );
    }


    ImGui::SameLine(
        0.0f,
        BackgroundGap
    );


    if (
        quietArrow(
            "##ModelBackgroundNext",
            ImGuiDir_Right
        )
    ) {
        cycleBackground(
            1
        );
    }
}


void ModelViewPanel::renderTransformControls(
    CacheExplorerState& state
) {
    eld::render::Transform& transform =
        state.modelTransform;


    float rotationDegrees[3]{
        radiansToDegrees(
            transform.rotation.x
        ),
        radiansToDegrees(
            transform.rotation.y
        ),
        radiansToDegrees(
            transform.rotation.z
        )
    };


    ImGui::SetNextItemWidth(
        -1.0f
    );

    if (
        ImGui::DragFloat3(
            "Rotation",
            rotationDegrees,
            0.5f,
            -360.0f,
            360.0f,
            "%.1f deg"
        )
    ) {
        transform.rotation = {
            degreesToRadians(
                rotationDegrees[0]
            ),
            degreesToRadians(
                rotationDegrees[1]
            ),
            degreesToRadians(
                rotationDegrees[2]
            )
        };
    }


    float position[3]{
        transform.position.x,
        transform.position.y,
        transform.position.z
    };


    ImGui::SetNextItemWidth(
        -1.0f
    );

    if (
        ImGui::DragFloat3(
            "Position",
            position,
            1.0f,
            0.0f,
            0.0f,
            "%.1f"
        )
    ) {
        transform.position = {
            position[0],
            position[1],
            position[2]
        };
    }


    float scale =
        transform.scale.x;


    ImGui::SetNextItemWidth(
        -1.0f
    );

    if (
        ImGui::DragFloat(
            "Scale",
            &scale,
            0.01f,
            0.1f,
            20.0f,
            "%.2fx"
        )
    ) {
        scale =
            std::max(
                scale,
                0.1f
            );

        transform.scale = {
            scale,
            scale,
            scale
        };
    }
}


void ModelViewPanel::renderViewPresets(
    CacheExplorerState& state
) {
    const auto presetButton =
        [this, &state](
            const char* label,
            ModelViewPreset preset
        ) {
            if (ImGui::Button(label)) {
                options_.viewPreset =
                    preset;

                options_.autoRotate =
                    false;

                applyViewPreset(
                    state
                );
            }
        };


    if (ImGui::Button("Reset")) {
        state.modelTransform = {};
        options_.autoRotate = false;
    }

    ImGui::SameLine();

    presetButton(
        "Iso",
        ModelViewPreset::Iso
    );

    ImGui::SameLine();

    presetButton(
        "Front",
        ModelViewPreset::Front
    );

    ImGui::SameLine();

    presetButton(
        "Back",
        ModelViewPreset::Back
    );


    presetButton(
        "Left",
        ModelViewPreset::Left
    );

    ImGui::SameLine();

    presetButton(
        "Right",
        ModelViewPreset::Right
    );

    ImGui::SameLine();

    presetButton(
        "Top",
        ModelViewPreset::Top
    );

    ImGui::SameLine();

    presetButton(
        "Bottom",
        ModelViewPreset::Bottom
    );
}

}
