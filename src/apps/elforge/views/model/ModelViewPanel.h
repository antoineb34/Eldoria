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


enum class ModelViewPreset {
    Iso,
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom
};


struct ModelViewOptions {
    bool showSolid = true;
    bool showWireframe = false;
    bool showVertices = false;
    bool showBounds = false;
    bool showAxes = false;

    bool autoRotate = false;

    float autoRotateSpeed =
        0.7f;

    ModelBackground background =
        ModelBackground::Neutral;

    ModelViewPreset viewPreset =
        ModelViewPreset::Iso;

    std::array<std::uint8_t, 4>
    backgroundColor() const;
};


class ModelViewPanel {
public:
    void update(
        CacheExplorerState& state,
        bool hasModel
    );

    // Existing embedded controls. Kept temporarily for
    // NPC/Location/SpotAnim workspaces until those are
    // standardized too.
    void render(
        CacheExplorerState& state,
        bool hasModel
    );

    // Canonical raw-model workspace.
    void renderWorkspace(
        CacheExplorerState& state,
        bool hasModel
    );

    const ModelViewOptions&
    options() const;


private:
    static constexpr float Pi =
        3.14159265358979323846f;

    static void setRotation(
        CacheExplorerState& state,
        float x,
        float y,
        float z = 0.0f
    );

    static float radiansToDegrees(
        float radians
    );

    static float degreesToRadians(
        float degrees
    );

    void applyViewPreset(
        CacheExplorerState& state
    );

    void cycleViewPreset(
        CacheExplorerState& state,
        int direction
    );

    void cycleBackground(
        int direction
    );

    void renderTransformControls(
        CacheExplorerState& state
    );

    void renderViewPresets(
        CacheExplorerState& state
    );

    void renderWorkspaceViewCard(
        CacheExplorerState& state,
        float width
    );

    void renderWorkspaceModelCard(
        const CacheExplorerState& state,
        float width
    );

    void renderWorkspaceDisplayCard(
        float width
    );


    ModelViewOptions options_;
};

}
