#include "ModelViewPanel.h"

#include <algorithm>

#include <imgui.h>

#include "../CacheExplorerState.h"

namespace eld::elforge {

std::array<std::uint8_t, 4>
ModelViewOptions::backgroundColor() const {
    switch (background) {
        case ModelBackground::Dark:
            return {28, 30, 34, 255};
        case ModelBackground::Light:
            return {185, 185, 185, 255};
        case ModelBackground::Neutral:
        default:
            return {68, 88, 68, 255};
    }
}

void ModelViewPanel::update(
    CacheExplorerState& state,
    bool hasModel
) {
    if (!hasModel || !options_.autoRotate) {
        return;
    }

    state.modelTransform.rotation.y +=
        options_.autoRotateSpeed *
        std::max(ImGui::GetIO().DeltaTime, 0.0f);
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

    ImGui::BeginDisabled(!hasModel);

    renderViewPresets(state);

    ImGui::Spacing();
    ImGui::TextUnformatted("TRANSFORM");
    ImGui::Separator();

    renderTransformControls(state);

    ImGui::Spacing();
    ImGui::TextUnformatted("DISPLAY");
    ImGui::Separator();

    ImGui::Checkbox("Solid", &options_.showSolid);
    ImGui::SameLine();
    ImGui::Checkbox("Wireframe", &options_.showWireframe);
    ImGui::SameLine();
    ImGui::Checkbox("Vertices", &options_.showVertices);

    ImGui::Checkbox("Bounds", &options_.showBounds);
    ImGui::SameLine();
    ImGui::Checkbox("Axes", &options_.showAxes);

    ImGui::Spacing();
    ImGui::TextUnformatted("MOTION");
    ImGui::Separator();

    ImGui::Checkbox("Auto rotate", &options_.autoRotate);

    if (options_.autoRotate) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderFloat(
            "##ModelAutoRotateSpeed",
            &options_.autoRotateSpeed,
            -3.0f,
            3.0f,
            "%.2f rad/s"
        );
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("BACKGROUND");
    ImGui::Separator();

    static const char* Backgrounds[] = {
        "Neutral",
        "Dark",
        "Light"
    };

    int background = static_cast<int>(options_.background);

    ImGui::SetNextItemWidth(140.0f);

    if (
        ImGui::Combo(
            "##ModelBackground",
            &background,
            Backgrounds,
            3
        )
    ) {
        options_.background =
            static_cast<ModelBackground>(background);
    }

    ImGui::EndDisabled();
}

const ModelViewOptions& ModelViewPanel::options() const {
    return options_;
}

void ModelViewPanel::setRotation(
    CacheExplorerState& state,
    float x,
    float y,
    float z
) {
    state.modelTransform.rotation = {x, y, z};
}

float ModelViewPanel::radiansToDegrees(float radians) {
    return radians * 180.0f / Pi;
}

float ModelViewPanel::degreesToRadians(float degrees) {
    return degrees * Pi / 180.0f;
}

void ModelViewPanel::renderTransformControls(
    CacheExplorerState& state
) {
    eld::render::Transform& transform = state.modelTransform;

    float rotationDegrees[3]{
        radiansToDegrees(transform.rotation.x),
        radiansToDegrees(transform.rotation.y),
        radiansToDegrees(transform.rotation.z)
    };

    ImGui::SetNextItemWidth(-1.0f);

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
            degreesToRadians(rotationDegrees[0]),
            degreesToRadians(rotationDegrees[1]),
            degreesToRadians(rotationDegrees[2])
        };
    }

    float position[3]{
        transform.position.x,
        transform.position.y,
        transform.position.z
    };

    ImGui::SetNextItemWidth(-1.0f);

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

    float scale = transform.scale.x;

    ImGui::SetNextItemWidth(-1.0f);

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
        scale = std::max(scale, 0.1f);
        transform.scale = {scale, scale, scale};
    }
}

void ModelViewPanel::renderViewPresets(
    CacheExplorerState& state
) {
    if (ImGui::Button("Reset")) {
        state.modelTransform = {};
    }

    ImGui::SameLine();

    if (ImGui::Button("Iso")) {
        setRotation(state, -0.55f, 0.78f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Front")) {
        setRotation(state, 0.0f, 0.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Back")) {
        setRotation(state, 0.0f, Pi);
    }

    if (ImGui::Button("Left")) {
        setRotation(state, 0.0f, -Pi * 0.5f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Right")) {
        setRotation(state, 0.0f, Pi * 0.5f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Top")) {
        setRotation(state, -Pi * 0.5f, 0.0f);
    }

    ImGui::SameLine();

    if (ImGui::Button("Bottom")) {
        setRotation(state, Pi * 0.5f, 0.0f);
    }
}

}
