#include "Color.h"

#include <algorithm>

namespace eld::render {

static float hueToRgb(
    float p,
    float q,
    float t
) {
    if (t < 0.0f) {
        t += 1.0f;
    }

    if (t > 1.0f) {
        t -= 1.0f;
    }

    if (t < 1.0f / 6.0f) {
        return
            p +
            (q - p) *
            6.0f *
            t;
    }

    if (t < 1.0f / 2.0f) {
        return q;
    }

    if (t < 2.0f / 3.0f) {
        return
            p +
            (q - p) *
            (2.0f / 3.0f - t) *
            6.0f;
    }

    return p;
}

RgbColor rsColorToRgb(
    uint16_t color
) {
    int hue =
        (color >> 10) & 0x3F;

    int saturation =
        (color >> 7) & 0x07;

    int lightness =
        color & 0x7F;

    float h =
        static_cast<float>(hue) /
        64.0f;

    float s =
        static_cast<float>(saturation) /
        7.0f;

    float l =
        static_cast<float>(lightness) /
        127.0f;

    float r = l;
    float g = l;
    float b = l;

    if (s != 0.0f) {
        float q =
            l < 0.5f
                ? l * (1.0f + s)
                : l + s - l * s;

        float p =
            2.0f * l - q;

        r =
            hueToRgb(
                p,
                q,
                h + 1.0f / 3.0f
            );

        g =
            hueToRgb(
                p,
                q,
                h
            );

        b =
            hueToRgb(
                p,
                q,
                h - 1.0f / 3.0f
            );
    }

    return {
        static_cast<uint8_t>(
            std::clamp(
                r * 255.0f,
                0.0f,
                255.0f
            )
        ),

        static_cast<uint8_t>(
            std::clamp(
                g * 255.0f,
                0.0f,
                255.0f
            )
        ),

        static_cast<uint8_t>(
            std::clamp(
                b * 255.0f,
                0.0f,
                255.0f
            )
        )
    };
}

}
