#pragma once

#include <algorithm>

namespace eld::elforge::viewport_workspace {

inline constexpr float BottomHeight =
    160.0f;

inline constexpr float FadeHeight =
    125.0f;

inline constexpr float HorizontalInset =
    34.0f;

inline constexpr float BottomInset =
    26.0f;

inline constexpr float ColumnGap =
    16.0f;

inline constexpr float CardHeight =
    92.0f;

inline constexpr float LeftWidth =
    270.0f;

inline constexpr float CenterPreferredWidth =
    250.0f;

inline constexpr float CenterMinimumWidth =
    210.0f;

inline constexpr float RightWidthRatio =
    0.36f;

inline constexpr float RightMinimumWidth =
    380.0f;

inline constexpr float RightMaximumWidth =
    490.0f;


struct BottomRow {
    float leftX = 0.0f;

    float centerX = 0.0f;
    float centerWidth = 0.0f;

    float rightX = 0.0f;
    float rightWidth = 0.0f;
};


[[nodiscard]] inline float preferredRightWidth(
    float viewportWidth
) noexcept {
    return std::clamp(
        viewportWidth *
            RightWidthRatio,
        RightMinimumWidth,
        RightMaximumWidth
    );
}


[[nodiscard]] inline BottomRow bottomRow(
    float viewportWidth
) noexcept {
    BottomRow result;

    result.leftX =
        HorizontalInset;


    const float availableWidth =
        std::max(
            viewportWidth -
                HorizontalInset *
                    2.0f -
                LeftWidth -
                ColumnGap *
                    2.0f,
            0.0f
        );


    const float desiredRightWidth =
        preferredRightWidth(
            viewportWidth
        );

    const float maximumRightWidth =
        std::max(
            availableWidth -
                CenterMinimumWidth,
            0.0f
        );


    result.rightWidth =
        std::min(
            desiredRightWidth,
            maximumRightWidth
        );

    result.rightX =
        std::max(
            viewportWidth -
                HorizontalInset -
                result.rightWidth,
            8.0f
        );


    const float middleStart =
        result.leftX +
        LeftWidth +
        ColumnGap;

    const float middleEnd =
        result.rightX -
        ColumnGap;

    const float middleWidth =
        std::max(
            middleEnd -
                middleStart,
            0.0f
        );


    result.centerWidth =
        std::min(
            CenterPreferredWidth,
            middleWidth
        );

    result.centerX =
        middleStart +
        std::max(
            (
                middleWidth -
                    result.centerWidth
            ) *
                0.5f,
            0.0f
        );


    return result;
}

}
