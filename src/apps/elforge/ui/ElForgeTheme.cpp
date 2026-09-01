#include "ui/ElForgeTheme.h"

namespace eld::elforge::ui {

namespace {

ElForgeTheme activeTheme =
    ElForgeTheme::Forest;


ImVec4 rgb(
    std::uint32_t hex,
    float alpha = 1.0f
) {
    return ImVec4(
        static_cast<float>(
            (hex >> 16) & 0xFF
        ) / 255.0f,
        static_cast<float>(
            (hex >> 8) & 0xFF
        ) / 255.0f,
        static_cast<float>(
            hex & 0xFF
        ) / 255.0f,
        alpha
    );
}


std::array<std::uint8_t, 4> rgba8(
    std::uint32_t hex,
    std::uint8_t alpha = 255
) {
    return {
        static_cast<std::uint8_t>(
            (hex >> 16) & 0xFF
        ),
        static_cast<std::uint8_t>(
            (hex >> 8) & 0xFF
        ),
        static_cast<std::uint8_t>(
            hex & 0xFF
        ),
        alpha
    };
}


ImVec4 alpha(
    ImVec4 value,
    float opacity
) {
    value.w =
        opacity;

    return value;
}


ElForgePalette palette(
    // foreground
    std::uint32_t text,
    std::uint32_t muted,

    // structural
    std::uint32_t window,
    std::uint32_t child,
    std::uint32_t popup,
    std::uint32_t border,
    std::uint32_t surface,
    std::uint32_t surfaceHovered,

    // identity
    std::uint32_t primary,
    std::uint32_t secondary,
    std::uint32_t tertiary,

    // semantic
    std::uint32_t positive,
    std::uint32_t warning,
    std::uint32_t danger,

    // viewport
    std::uint32_t hud,
    std::uint32_t viewport,
    std::uint32_t gridMajor,
    std::uint32_t gridMinor
) {
    return {
        .text =
            rgb(text),

        .textDisabled =
            rgb(muted),

        .window =
            rgb(window),

        .child =
            rgb(child),

        .popup =
            rgb(
                popup,
                0.985f
            ),

        .border =
            rgb(
                border,
                0.76f
            ),

        .surface =
            rgb(surface),

        .surfaceHovered =
            rgb(surfaceHovered),

        .primary =
            rgb(primary),

        .secondary =
            rgb(secondary),

        .tertiary =
            rgb(tertiary),

        .positive =
            rgb(positive),

        .warning =
            rgb(warning),

        .danger =
            rgb(danger),

        .hudBackground =
            rgb(
                hud,
                1.0f
            ),

        .viewportBackground =
            rgba8(viewport),

        .gridMajor =
            rgba8(
                gridMajor,
                110
            ),

        .gridMinor =
            rgba8(
                gridMinor,
                58
            )
    };
}


// ============================================================
// FOREST
//
// Everforest dark / medium.
//
// UI:
//   bg_dim #232A2E
//   bg0    #2D353B
//   bg1    #343F44
//
// Viewport:
//   bg_blue #3A515D
//
// This separation is exactly why Forest works so well.
// ============================================================

const ElForgePalette Forest =
    palette(
        0xE5E3DA, // text - crisp warm ivory
        0xB8BAAE, // muted text - readable warm gray

        0x0E1010, // window
        0x151716, // panels / explorer
        0x1B1C1B, // popup
        0x4A5047, // border - slightly sharper
        0x1F201E, // raised surface
        0x2B2E28, // surface hover

        0xB6C760, // primary - punchy khaki green
        0x9EAF69, // secondary - olive green
        0xD0C078, // tertiary - warm khaki

        0xA6B96B, // positive
        0xC99D61, // warning
        0xD07064, // danger

        0x191C1B, // HUD

        0x9AAF94, // viewport
        0x788474, // major grid
        0x687367  // minor grid
    );


// ============================================================
// CATPPUCCIN MOCHA
//
// UI uses Crust / Mantle / Surface.
// Viewport uses Base.
// ============================================================

const ElForgePalette CatppuccinMocha =
    palette(
        0xCDD6F4, // text
        0x7F849C, // overlay1

        0x11111B, // crust
        0x181825, // mantle
        0x1E1E2E, // base popup
        0x585B70, // surface2
        0x313244, // surface0
        0x45475A, // surface1

        0x89B4FA, // blue
        0xCBA6F7, // mauve
        0xFAB387, // peach

        0xA6E3A1, // green
        0xF9E2AF, // yellow
        0xF38BA8, // red

        0x11111B, // HUD
        0x1E1E2E, // base viewport
        0x6C7086, // overlay0
        0x45475A  // surface1
    );


// ============================================================
// TOKYO NIGHT STORM
//
// Official Storm:
// bg_dark1   #1B1E2D
// bg_dark    #1F2335
// bg         #24283B
// bg_highlight #292E42
// bg_visual  #2E3C64
// ============================================================

const ElForgePalette TokyoNightStorm =
    palette(
        0xC0CAF5, // fg
        0x565F89, // comment

        0x1B1E2D, // bg_dark1
        0x1F2335, // bg_dark/sidebar
        0x1F2335, // popup
        0x3B4261, // fg_gutter
        0x292E42, // bg_highlight
        0x2E3C64, // bg_visual

        0x7AA2F7, // blue
        0x7DCFFF, // cyan
        0xBB9AF7, // magenta

        0x9ECE6A, // green
        0xE0AF68, // yellow
        0xF7768E, // red

        0x1B1E2D, // HUD
        0x24283B, // bg viewport
        0x394B70, // blue7
        0x414868  // terminal black
    );


// ============================================================
// GRUVBOX DARK
//
// Uses its actual warm dark backgrounds.
// Viewport uses dark0_soft rather than normal UI dark0.
// ============================================================

const ElForgePalette GruvboxDark =
    palette(
        0xEBDBB2, // light1
        0x928374, // gray

        0x1D2021, // dark0_hard
        0x282828, // dark0
        0x32302F, // dark0_soft
        0x665C54, // dark3
        0x3C3836, // dark1
        0x504945, // dark2

        0x83A598, // bright blue
        0x8EC07C, // bright aqua
        0xFE8019, // bright orange

        0xB8BB26, // bright green
        0xFABD2F, // bright yellow
        0xFB4934, // bright red

        0x1D2021, // HUD
        0x32302F, // dark0_soft viewport
        0x7C6F64, // dark4
        0x665C54  // dark3
    );


// ============================================================
// ROSE PINE MOON
//
// base     #232136
// surface  #2A273F
// overlay  #393552
//
// We deliberately use Overlay for the viewport so it is
// visibly its own canvas.
// ============================================================

const ElForgePalette RosePineMoon =
    palette(
        0xE0DEF4, // text
        0x6E6A86, // muted

        0x1F1D30, // _nc
        0x232136, // base
        0x2A273F, // surface
        0x56526E, // highlight high
        0x2A273F, // surface
        0x44415A, // highlight med

        0x3E8FB0, // pine
        0x9CCFD8, // foam
        0xC4A7E7, // iris

        0x95B1AC, // leaf
        0xF6C177, // gold
        0xEB6F92, // love

        0x1F1D30, // HUD
        0x393552, // overlay viewport
        0x908CAA, // subtle
        0x6E6A86  // muted
    );


const ElForgePalette& paletteFor(
    ElForgeTheme theme
) {
    switch (theme) {
        case ElForgeTheme::Forest:
            return Forest;

        case ElForgeTheme::CatppuccinMocha:
            return CatppuccinMocha;

        case ElForgeTheme::TokyoNightStorm:
            return TokyoNightStorm;

        case ElForgeTheme::GruvboxDark:
            return GruvboxDark;

        case ElForgeTheme::RosePineMoon:
            return RosePineMoon;

        case ElForgeTheme::Count:
            break;
    }

    return Forest;
}


void applyGeometry(
    ImGuiStyle& style
) {
    style.WindowPadding =
        ImVec2(10.0f, 8.0f);

    style.FramePadding =
        ImVec2(8.0f, 5.0f);

    style.CellPadding =
        ImVec2(7.0f, 5.0f);

    style.ItemSpacing =
        ImVec2(7.0f, 6.0f);

    style.ItemInnerSpacing =
        ImVec2(6.0f, 4.0f);

    style.IndentSpacing =
        16.0f;

    style.ScrollbarSize =
        11.0f;

    style.GrabMinSize =
        10.0f;

    style.WindowRounding =
        0.0f;

    style.ChildRounding =
        6.0f;

    style.FrameRounding =
        4.0f;

    style.PopupRounding =
        6.0f;

    style.ScrollbarRounding =
        4.0f;

    style.GrabRounding =
        3.0f;

    style.TabRounding =
        4.0f;

    style.WindowBorderSize =
        0.0f;

    style.ChildBorderSize =
        1.0f;

    style.PopupBorderSize =
        1.0f;

    style.FrameBorderSize =
        1.0f;
}


void applyColors(
    ImGuiStyle& style,
    const ElForgePalette& p
) {
    ImVec4* c =
        style.Colors;

    // Foundation
    c[ImGuiCol_Text] =
        p.text;

    c[ImGuiCol_TextDisabled] =
        p.textDisabled;

    c[ImGuiCol_WindowBg] =
        p.window;

    c[ImGuiCol_ChildBg] =
        p.child;

    c[ImGuiCol_PopupBg] =
        p.popup;

    c[ImGuiCol_Border] =
        alpha(
            p.border,
            0.72f
        );

    c[ImGuiCol_BorderShadow] =
        ImVec4(0, 0, 0, 0);


    // Inputs / buttons:
    // neutral graphite at rest, warm theme color only
    // when the control actually has emphasis.
    c[ImGuiCol_FrameBg] =
        p.surface;

    c[ImGuiCol_FrameBgHovered] =
        p.surfaceHovered;

    c[ImGuiCol_FrameBgActive] =
        alpha(
            p.primary,
            0.24f
        );

    c[ImGuiCol_Button] =
        p.surface;

    c[ImGuiCol_ButtonHovered] =
        p.surfaceHovered;

    c[ImGuiCol_ButtonActive] =
        alpha(
            p.primary,
            0.32f
        );


    // Selection.
    c[ImGuiCol_Header] =
        alpha(
            p.primary,
            0.31f
        );

    c[ImGuiCol_HeaderHovered] =
        alpha(
            p.primary,
            0.37f
        );

    c[ImGuiCol_HeaderActive] =
        alpha(
            p.primary,
            0.45f
        );

    c[ImGuiCol_TextSelectedBg] =
        alpha(
            p.primary,
            0.34f
        );


    // Semantic controls.
    c[ImGuiCol_CheckMark] =
        p.primary;

    c[ImGuiCol_SliderGrab] =
        p.tertiary;

    c[ImGuiCol_SliderGrabActive] =
        p.warning;


    // Separators.
    c[ImGuiCol_Separator] =
        alpha(
            p.border,
            0.55f
        );

    c[ImGuiCol_SeparatorHovered] =
        p.secondary;

    c[ImGuiCol_SeparatorActive] =
        p.tertiary;


    // Scroll / resize.
    c[ImGuiCol_ScrollbarBg] =
        p.window;

    c[ImGuiCol_ScrollbarGrab] =
        p.surface;

    c[ImGuiCol_ScrollbarGrabHovered] =
        p.surfaceHovered;

    c[ImGuiCol_ScrollbarGrabActive] =
        alpha(
            p.secondary,
            0.72f
        );

    c[ImGuiCol_ResizeGrip] =
        alpha(
            p.tertiary,
            0.14f
        );

    c[ImGuiCol_ResizeGripHovered] =
        alpha(
            p.tertiary,
            0.42f
        );

    c[ImGuiCol_ResizeGripActive] =
        p.tertiary;


    // Tabs.
    c[ImGuiCol_Tab] =
        p.surface;

    c[ImGuiCol_TabHovered] =
        p.surfaceHovered;

    c[ImGuiCol_TabSelected] =
        alpha(
            p.primary,
            0.18f
        );


    // Tables.
    c[ImGuiCol_TableHeaderBg] =
        p.surface;

    c[ImGuiCol_TableBorderStrong] =
        p.border;

    c[ImGuiCol_TableBorderLight] =
        alpha(
            p.border,
            0.42f
        );

    c[ImGuiCol_TableRowBg] =
        ImVec4(0, 0, 0, 0);

    c[ImGuiCol_TableRowBgAlt] =
        alpha(
            p.surface,
            0.40f
        );


    // Window chrome.
    c[ImGuiCol_TitleBg] =
        p.window;

    c[ImGuiCol_TitleBgActive] =
        p.child;

    c[ImGuiCol_TitleBgCollapsed] =
        p.window;

    c[ImGuiCol_MenuBarBg] =
        p.child;


    // Interaction / data.
    c[ImGuiCol_DragDropTarget] =
        p.tertiary;

    c[ImGuiCol_NavHighlight] =
        p.primary;

    c[ImGuiCol_PlotLines] =
        p.secondary;

    c[ImGuiCol_PlotLinesHovered] =
        p.tertiary;

    c[ImGuiCol_PlotHistogram] =
        p.primary;

    c[ImGuiCol_PlotHistogramHovered] =
        p.warning;


    // Dark overlay should stay neutral, not colored.
    c[ImGuiCol_ModalWindowDimBg] =
        ImVec4(
            0.0f,
            0.0f,
            0.0f,
            0.72f
        );

    c[ImGuiCol_NavWindowingDimBg] =
        ImVec4(
            0.0f,
            0.0f,
            0.0f,
            0.55f
        );
}

}


const ElForgePalette& themePalette() {
    return paletteFor(
        activeTheme
    );
}


void applyElForgeTheme(
    ElForgeTheme theme
) {
    if (
        theme ==
        ElForgeTheme::Count
    ) {
        theme =
            ElForgeTheme::Forest;
    }

    activeTheme =
        theme;

    ImGuiStyle& style =
        ImGui::GetStyle();

    applyGeometry(style);

    applyColors(
        style,
        themePalette()
    );
}


void setElForgeTheme(
    ElForgeTheme theme
) {
    applyElForgeTheme(
        theme
    );
}


void cycleElForgeTheme(
    int direction
) {
    const int count =
        static_cast<int>(
            ElForgeTheme::Count
        );

    int value =
        static_cast<int>(
            activeTheme
        );

    value =
        (
            value +
            direction
        ) %
        count;

    if (value < 0) {
        value +=
            count;
    }

    applyElForgeTheme(
        static_cast<ElForgeTheme>(
            value
        )
    );
}


ElForgeTheme currentElForgeTheme() {
    return activeTheme;
}


const char* elForgeThemeName(
    ElForgeTheme theme
) {
    switch (theme) {
        case ElForgeTheme::Forest:
            return "Forest";

        case ElForgeTheme::CatppuccinMocha:
            return "Catppuccin Mocha";

        case ElForgeTheme::TokyoNightStorm:
            return "Tokyo Night Storm";

        case ElForgeTheme::GruvboxDark:
            return "Gruvbox Dark";

        case ElForgeTheme::RosePineMoon:
            return "Rose Pine Moon";

        case ElForgeTheme::Count:
            break;
    }

    return "Forest";
}

}
