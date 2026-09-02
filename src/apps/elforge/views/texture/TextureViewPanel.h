#pragma once

#include <imgui.h>

namespace eld::elforge {

struct CacheExplorerState;

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

    TextureZoomMode zoomMode = TextureZoomMode::Fit;
    bool nearestSampling = true;

    bool showRed = true;
    bool showGreen = true;
    bool showBlue = true;
    bool showAlpha = true;
    bool alphaOnly = false;

    bool showBorder = true;
    bool showPixelGrid = false;

    float fixedScale() const;
};

class TextureViewPanel {
public:
    void render(bool hasTexture);

    void renderWorkspace(
        CacheExplorerState& state,
        bool hasTexture,
        const ImVec2& controlsPosition,
        const ImVec2& controlsSize
    );

    const TextureViewOptions& options() const;

private:
    void renderPresets();

    TextureViewOptions options_;
};

}
