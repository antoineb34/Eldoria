#pragma once

#include <array>
#include <cstdint>

namespace eld::elforge {

struct CacheExplorerState;

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

    ModelBackground background = ModelBackground::Neutral;

    std::array<std::uint8_t, 4> backgroundColor() const;
};

class ModelViewPanel {
public:
    void update(
        CacheExplorerState& state,
        bool hasModel
    );

    void render(
        CacheExplorerState& state,
        bool hasModel
    );

    const ModelViewOptions& options() const;

private:
    static constexpr float Pi =
        3.14159265358979323846f;

    static void setRotation(
        CacheExplorerState& state,
        float x,
        float y,
        float z = 0.0f
    );

    static float radiansToDegrees(float radians);
    static float degreesToRadians(float degrees);

    void renderTransformControls(CacheExplorerState& state);
    void renderViewPresets(CacheExplorerState& state);

    ModelViewOptions options_;
};

}
