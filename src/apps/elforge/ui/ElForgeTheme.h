#pragma once

#include <array>
#include <cstdint>

#include <imgui.h>

namespace eld::elforge::ui {

enum class ElForgeTheme {
    Forest,
    CatppuccinMocha,
    TokyoNightStorm,
    GruvboxDark,
    RosePineMoon,

    Count
};


struct ElForgePalette {
    // Typography
    ImVec4 text;
    ImVec4 textDisabled;

    // Structural layers
    ImVec4 window;
    ImVec4 child;
    ImVec4 popup;
    ImVec4 border;

    ImVec4 surface;
    ImVec4 surfaceHovered;

    // Actual color identity
    ImVec4 primary;
    ImVec4 secondary;
    ImVec4 tertiary;

    ImVec4 positive;
    ImVec4 warning;
    ImVec4 danger;

    // Floating viewport cards
    ImVec4 hudBackground;

    // Renderer-backed workspace canvas.
    // Intentionally independent from window/child.
    std::array<std::uint8_t, 4>
        viewportBackground;

    std::array<std::uint8_t, 4>
        gridMajor;

    std::array<std::uint8_t, 4>
        gridMinor;
};


void applyElForgeTheme(
    ElForgeTheme theme
);

void setElForgeTheme(
    ElForgeTheme theme
);

void cycleElForgeTheme(
    int direction = 1
);

ElForgeTheme currentElForgeTheme();

const char* elForgeThemeName(
    ElForgeTheme theme
);

const ElForgePalette& themePalette();

}
