#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>

namespace rf::render {

static void drawScanline(
    SDL_Renderer* renderer,
    float x1,
    float x2,
    int y
) {
    if (x1 > x2) {
        std::swap(x1, x2);
    }

    SDL_RenderLine(
        renderer,
        x1,
        static_cast<float>(y),
        x2,
        static_cast<float>(y)
    );
}

static void drawFlatBottomTriangle(
    SDL_Renderer* renderer,
    ScreenPoint top,
    ScreenPoint left,
    ScreenPoint right
) {
    if (
        std::abs(left.y - top.y) < 0.0001f ||
        std::abs(right.y - top.y) < 0.0001f
    ) {
        return;
    }

    float invSlopeLeft =
        (left.x - top.x) /
        (left.y - top.y);

    float invSlopeRight =
        (right.x - top.x) /
        (right.y - top.y);

    float currentLeftX = top.x;
    float currentRightX = top.x;

    int startY =
        static_cast<int>(std::ceil(top.y));

    int endY =
        static_cast<int>(std::floor(left.y));

    for (int y = startY; y <= endY; y++) {
        drawScanline(
            renderer,
            currentLeftX,
            currentRightX,
            y
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
    if (
        std::abs(bottom.y - left.y) < 0.0001f ||
        std::abs(bottom.y - right.y) < 0.0001f
    ) {
        return;
    }

    float invSlopeLeft =
        (bottom.x - left.x) /
        (bottom.y - left.y);

    float invSlopeRight =
        (bottom.x - right.x) /
        (bottom.y - right.y);

    float currentLeftX = bottom.x;
    float currentRightX = bottom.x;

    int startY =
        static_cast<int>(std::floor(bottom.y));

    int endY =
        static_cast<int>(std::ceil(left.y));

    for (int y = startY; y > endY; y--) {
        drawScanline(
            renderer,
            currentLeftX,
            currentRightX,
            y
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

    if (std::abs(v2.y - v0.y) < 0.0001f) {
        return;
    }

    if (std::abs(v1.y - v2.y) < 0.0001f) {
        drawFlatBottomTriangle(
            renderer,
            v0,
            v1,
            v2
        );

        return;
    }

    if (std::abs(v0.y - v1.y) < 0.0001f) {
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
