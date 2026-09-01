#pragma once

#include <algorithm>

namespace eld::elforge::animation_hud {

inline constexpr float BottomHeight = 160.0f;
inline constexpr float FadeHeight = 125.0f;

inline constexpr float HorizontalInset = 34.0f;
inline constexpr float BottomInset = 26.0f;
inline constexpr float ColumnGap = 16.0f;

inline constexpr float CardHeight = 92.0f;
inline constexpr float TransportWidth = 270.0f;

inline constexpr float FramePreferredWidth = 250.0f;
inline constexpr float FrameMinimumWidth = 210.0f;

inline constexpr float ContextWidthRatio = 0.36f;
inline constexpr float ContextMinimumWidth = 380.0f;
inline constexpr float ContextMaximumWidth = 490.0f;

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
    return std::clamp(
        viewportWidth * ContextWidthRatio,
        ContextMinimumWidth,
        ContextMaximumWidth
    );
}

[[nodiscard]] inline BottomRow bottomRow(
    float viewportWidth
) noexcept {
    BottomRow result;

    result.transportX =
        HorizontalInset;


    // --------------------------------------------------------
    // Available width for FRAME + CONTEXT.
    //
    // Playback owns its fixed 270px.
    // The two column gaps are also reserved here.
    // --------------------------------------------------------

    const float availableWidth =
        std::max(
            viewportWidth -
                HorizontalInset * 2.0f -
                TransportWidth -
                ColumnGap * 2.0f,
            0.0f
        );


    // --------------------------------------------------------
    // Context is flexible.
    //
    // Start with its preferred responsive width, but never
    // allow it to consume the space reserved for FRAME.
    // --------------------------------------------------------

    const float desiredContextWidth =
        contextWidth(
            viewportWidth
        );

    const float maximumContextWidth =
        std::max(
            availableWidth -
                FrameMinimumWidth,
            0.0f
        );

    result.contextWidth =
        std::min(
            desiredContextWidth,
            maximumContextWidth
        );

    result.contextX =
        std::max(
            viewportWidth -
                HorizontalInset -
                result.contextWidth,
            8.0f
        );


    // --------------------------------------------------------
    // Frame gets the space between Playback and Context.
    // It prefers 250px but can gracefully shrink to its
    // protected minimum.
    // --------------------------------------------------------

    const float middleStart =
        result.transportX +
        TransportWidth +
        ColumnGap;

    const float middleEnd =
        result.contextX -
        ColumnGap;

    const float middleWidth =
        std::max(
            middleEnd -
                middleStart,
            0.0f
        );

    result.frameWidth =
        std::min(
            FramePreferredWidth,
            middleWidth
        );

    result.frameX =
        middleStart +
        std::max(
            (
                middleWidth -
                    result.frameWidth
            ) *
                0.5f,
            0.0f
        );

    return result;
}

}
