#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include <imgui.h>

#include "../CacheExplorerState.h"

namespace eld::elforge {

enum class ModelBackground {
    Neutral,
    Dark,
    Light
};

struct ModelViewOptions {
    bool showSolid = true;
    bool showWireframe = false;
    bool showVertices = false;
    bool showBounds = false;
    bool showAxes = false;

    bool autoRotate = false;
    float autoRotateSpeed = 0.7f;

    ModelBackground background =
        ModelBackground::Neutral;

    std::array<std::uint8_t, 4>
    backgroundColor() const {
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
};

class ModelViewPanel {
public:
    void update(
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

    void render(
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

        ImGui::Spacing();
        ImGui::TextUnformatted("DISPLAY");
        ImGui::Separator();

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

        ImGui::Spacing();
        ImGui::TextUnformatted("MOTION");
        ImGui::Separator();

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

        ImGui::Spacing();
        ImGui::TextUnformatted("BACKGROUND");
        ImGui::Separator();

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

    const ModelViewOptions& options() const {
        return options_;
    }

private:
    static constexpr float Pi =
        3.14159265358979323846f;

    static void setRotation(
        CacheExplorerState& state,
        float x,
        float y,
        float z = 0.0f
    ) {
        state.modelTransform.rotation = {
            x,
            y,
            z
        };
    }

    void renderViewPresets(
        CacheExplorerState& state
    ) {
        if (ImGui::Button("Reset")) {
            state.modelTransform = {};
        }

        ImGui::SameLine();

        if (ImGui::Button("Iso")) {
            setRotation(
                state,
                -0.55f,
                0.78f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Front")) {
            setRotation(
                state,
                0.0f,
                0.0f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Back")) {
            setRotation(
                state,
                0.0f,
                Pi
            );
        }

        if (ImGui::Button("Left")) {
            setRotation(
                state,
                0.0f,
                -Pi * 0.5f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Right")) {
            setRotation(
                state,
                0.0f,
                Pi * 0.5f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Top")) {
            setRotation(
                state,
                -Pi * 0.5f,
                0.0f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Bottom")) {
            setRotation(
                state,
                Pi * 0.5f,
                0.0f
            );
        }
    }

    ModelViewOptions options_;
};

}
