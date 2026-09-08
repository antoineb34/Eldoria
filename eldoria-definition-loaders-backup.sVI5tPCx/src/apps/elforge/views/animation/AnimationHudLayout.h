#pragma once

#include "viewport/ViewportWorkspaceLayout.h"

namespace eld::elforge::animation_hud {

inline constexpr float BottomHeight =
    viewport_workspace::BottomHeight;

inline constexpr float FadeHeight =
    viewport_workspace::FadeHeight;

inline constexpr float HorizontalInset =
    viewport_workspace::HorizontalInset;

inline constexpr float BottomInset =
    viewport_workspace::BottomInset;

inline constexpr float ColumnGap =
    viewport_workspace::ColumnGap;

inline constexpr float CardHeight =
    viewport_workspace::CardHeight;

inline constexpr float TransportWidth =
    viewport_workspace::LeftWidth;

inline constexpr float FramePreferredWidth =
    viewport_workspace::CenterPreferredWidth;

inline constexpr float FrameMinimumWidth =
    viewport_workspace::CenterMinimumWidth;

inline constexpr float ContextWidthRatio =
    viewport_workspace::RightWidthRatio;

inline constexpr float ContextMinimumWidth =
    viewport_workspace::RightMinimumWidth;

inline constexpr float ContextMaximumWidth =
    viewport_workspace::RightMaximumWidth;


struct BottomRow {
    float transportX = 0.0f;

    float frameX = 0.0f;
    float frameWidth = 0.0f;

    float contextX = 0.0f;
    float contextWidth = 0.0f;
};


[[nodiscard]] inline float contextWidth(
    float viewportWidth
) noexcept {
    return
        viewport_workspace::
            preferredRightWidth(
                viewportWidth
            );
}


[[nodiscard]] inline BottomRow bottomRow(
    float viewportWidth
) noexcept {
    const viewport_workspace::BottomRow row =
        viewport_workspace::bottomRow(
            viewportWidth
        );

    return {
        row.leftX,
        row.centerX,
        row.centerWidth,
        row.rightX,
        row.rightWidth
    };
}

}
