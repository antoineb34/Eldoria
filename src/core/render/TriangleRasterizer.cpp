#include "TriangleRasterizer.h"

#include <algorithm>

namespace rf::render {

static void drawFlatBottomTriangle(
    SDL_Renderer* renderer,
    ScreenPoint top,
    ScreenPoint left,
    ScreenPoint right
) {
    float invSlopeLeft =
        (left.x - top.x) /
        (left.y - top.y);

    float invSlopeRight =
        (right.x - top.x) /
        (right.y - top.y);

    float currentLeftX =
        top.x;

    float currentRightX =
        top.x;

    for (
        int y = static_cast<int>(top.y);
        y <= static_cast<int>(left.y);
        y++
    ) {
        SDL_RenderLine(
            renderer,
            currentLeftX,
            static_cast<float>(y),
            currentRightX,
            static_cast<float>(y)
        );

        currentLeftX += invSlopeLeft;
        currentRightX += invSlopeRight;
    }
}

static void drawFlatTopTriangle(
    SDL_Renderer* renderer,
    ScreenPoint left,
    ScreenPoint right,
    ScreenPoint bottom
) {
    float invSlopeLeft =
        (bottom.x - left.x) /
        (bottom.y - left.y);

    float invSlopeRight =
        (bottom.x - right.x) /
        (bottom.y - right.y);

    float currentLeftX =
        bottom.x;

    float currentRightX =
        bottom.x;

    for (
        int y = static_cast<int>(bottom.y);
        y > static_cast<int>(left.y);
        y--
    ) {
        SDL_RenderLine(
            renderer,
            currentLeftX,
            static_cast<float>(y),
            currentRightX,
            static_cast<float>(y)
        );

        currentLeftX -= invSlopeLeft;
        currentRightX -= invSlopeRight;
    }
}

void fillTriangle(
    SDL_Renderer* renderer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
) {
    ScreenPoint v0 = a;
    ScreenPoint v1 = b;
    ScreenPoint v2 = c;

    if (v1.y < v0.y) {
        std::swap(v0, v1);
    }

    if (v2.y < v0.y) {
        std::swap(v0, v2);
    }

    if (v2.y < v1.y) {
        std::swap(v1, v2);
    }

    if (v1.y == v2.y) {
        drawFlatBottomTriangle(
            renderer,
            v0,
            v1,
            v2
        );

        return;
    }

    if (v0.y == v1.y) {
        drawFlatTopTriangle(
            renderer,
            v0,
            v1,
            v2
        );

        return;
    }

    float splitFactor =
        (v1.y - v0.y) /
        (v2.y - v0.y);

    ScreenPoint split {};

    split.x =
        v0.x +
        (v2.x - v0.x) *
        splitFactor;

    split.y =
        v1.y;

    split.z =
        v0.z +
        (v2.z - v0.z) *
        splitFactor;

    drawFlatBottomTriangle(
        renderer,
        v0,
        v1,
        split
    );

    drawFlatTopTriangle(
        renderer,
        v1,
        split,
        v2
    );
}

}
