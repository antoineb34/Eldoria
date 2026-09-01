#include "ClassicTerrainAppearance.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace eld::graphics::map {
namespace {

constexpr int InvisibleShade = 12345678;
constexpr double ClassicGamma = 0.8;

struct FloorHsl {
    int hue = 0;
    int saturation = 0;
    int lightness = 0;
    int chroma = 0;
    int luminance = 1;
};

std::uint32_t fallbackFloorRgb(
    std::uint8_t rawId
) {
    const std::uint32_t value =
        static_cast<std::uint32_t>(rawId) *
        2654435761u;

    const std::uint32_t r = 64u + ((value >> 16) & 0x7Fu);
    const std::uint32_t g = 64u + ((value >> 8) & 0x7Fu);
    const std::uint32_t b = 64u + (value & 0x7Fu);
    return (r << 16) | (g << 8) | b;
}

std::uint32_t floorRgb(
    std::uint8_t rawId,
    const eld::definition::FloorRepository& floors
) {
    if (rawId == 0) {
        return 0;
    }

    const auto* floor = floors.find(
        static_cast<std::uint16_t>(rawId - 1u)
    );
    if (floor == nullptr) {
        return fallbackFloorRgb(rawId);
    }
    if (floor->rgb.has_value()) {
        return *floor->rgb;
    }
    if (floor->secondaryRgb.has_value()) {
        return *floor->secondaryRgb;
    }
    return fallbackFloorRgb(rawId);
}

FloorHsl rgbToClassicHsl(
    std::uint32_t rgb
) {
    const double red =
        static_cast<double>((rgb >> 16) & 0xFFu) / 256.0;
    const double green =
        static_cast<double>((rgb >> 8) & 0xFFu) / 256.0;
    const double blue =
        static_cast<double>(rgb & 0xFFu) / 256.0;

    const double minimum = std::min({red, green, blue});
    const double maximum = std::max({red, green, blue});

    double h = 0.0;
    double s = 0.0;
    const double l = (minimum + maximum) / 2.0;

    if (minimum != maximum) {
        if (l < 0.5) {
            s = (maximum - minimum) / (maximum + minimum);
        }
        else {
            s =
                (maximum - minimum) /
                (2.0 - maximum - minimum);
        }

        if (red == maximum) {
            h = (green - blue) / (maximum - minimum);
        }
        else if (green == maximum) {
            h = (blue - red) / (maximum - minimum) + 2.0;
        }
        else {
            h = (red - green) / (maximum - minimum) + 4.0;
        }
    }

    h /= 6.0;

    FloorHsl result;
    result.hue = static_cast<int>(h * 256.0);
    result.saturation = std::clamp(
        static_cast<int>(s * 256.0),
        0,
        255
    );
    result.lightness = std::clamp(
        static_cast<int>(l * 256.0),
        0,
        255
    );

    result.luminance =
        l > 0.5
            ? static_cast<int>((1.0 - l) * s * 512.0)
            : static_cast<int>(l * s * 512.0);
    result.luminance = std::max(result.luminance, 1);
    result.chroma =
        static_cast<int>(h * static_cast<double>(result.luminance));

    return result;
}

int hsl24to16(
    int hue,
    int saturation,
    int lightness
) {
    if (lightness > 179) {
        saturation /= 2;
    }
    if (lightness > 192) {
        saturation /= 2;
    }
    if (lightness > 217) {
        saturation /= 2;
    }
    if (lightness > 243) {
        saturation /= 2;
    }

    return
        (hue / 4) * 1024 +
        (saturation / 32) * 128 +
        lightness / 2;
}

int mulHsl(
    int hsl,
    int lightness
) {
    if (hsl == -1) {
        return InvisibleShade;
    }

    const int low =
        static_cast<int>(
            static_cast<std::uint32_t>(hsl) & 0x7Fu
        );
    const int high =
        static_cast<int>(
            static_cast<std::uint32_t>(hsl) & 0xFF80u
        );

    lightness = lightness * low / 128;
    lightness = std::clamp(lightness, 2, 126);
    return high + lightness;
}

int adjustLightness(
    int hsl,
    int scalar
) {
    if (hsl == -2) {
        return InvisibleShade;
    }

    if (hsl == -1) {
        scalar = std::clamp(scalar, 0, 127);
        return 127 - scalar;
    }

    const int low =
        static_cast<int>(
            static_cast<std::uint32_t>(hsl) & 0x7Fu
        );
    const int high =
        static_cast<int>(
            static_cast<std::uint32_t>(hsl) & 0xFF80u
        );

    scalar = scalar * low / 128;
    scalar = std::clamp(scalar, 2, 126);
    return high + scalar;
}

std::uint32_t setGamma(
    std::uint32_t rgb,
    double gamma
) {
    const double r =
        static_cast<double>((rgb >> 16) & 0xFFu) / 256.0;
    const double g =
        static_cast<double>((rgb >> 8) & 0xFFu) / 256.0;
    const double b =
        static_cast<double>(rgb & 0xFFu) / 256.0;

    const std::uint32_t outR =
        static_cast<std::uint32_t>(std::pow(r, gamma) * 256.0);
    const std::uint32_t outG =
        static_cast<std::uint32_t>(std::pow(g, gamma) * 256.0);
    const std::uint32_t outB =
        static_cast<std::uint32_t>(std::pow(b, gamma) * 256.0);

    return
        ((outR & 0xFFu) << 16) |
        ((outG & 0xFFu) << 8) |
        (outB & 0xFFu);
}

double hueToRgb(
    double p,
    double q,
    double t
) {
    if (t < 0.0) {
        t += 1.0;
    }
    if (t > 1.0) {
        t -= 1.0;
    }
    if (t * 6.0 < 1.0) {
        return p + (q - p) * 6.0 * t;
    }
    if (t * 2.0 < 1.0) {
        return q;
    }
    if (t * 3.0 < 2.0) {
        return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    }
    return p;
}

const std::array<std::uint32_t, 65536>& classicPalette() {
    static const std::array<std::uint32_t, 65536> Palette = [] {
        std::array<std::uint32_t, 65536> palette{};
        std::size_t offset = 0;

        // The live client adds a tiny random gamma jitter. A deterministic
        // tool uses the nominal 0.8 client brightness so identical cache data
        // produces identical probe output every run.
        for (int y = 0; y < 512; ++y) {
            const double hue =
                static_cast<double>(y / 8) / 64.0 +
                0.0078125;
            const double saturation =
                static_cast<double>(y & 7) / 8.0 +
                0.0625;

            for (int x = 0; x < 128; ++x) {
                const double lightness =
                    static_cast<double>(x) / 128.0;

                double r = lightness;
                double g = lightness;
                double b = lightness;

                if (saturation != 0.0) {
                    const double q =
                        lightness < 0.5
                            ? lightness * (saturation + 1.0)
                            : lightness + saturation -
                                  lightness * saturation;
                    const double p = lightness * 2.0 - q;

                    r = hueToRgb(p, q, hue + 1.0 / 3.0);
                    g = hueToRgb(p, q, hue);
                    b = hueToRgb(p, q, hue - 1.0 / 3.0);
                }

                const std::uint32_t intR =
                    static_cast<std::uint32_t>(r * 256.0);
                const std::uint32_t intG =
                    static_cast<std::uint32_t>(g * 256.0);
                const std::uint32_t intB =
                    static_cast<std::uint32_t>(b * 256.0);
                const std::uint32_t rgb =
                    ((intR & 0xFFu) << 16) |
                    ((intG & 0xFFu) << 8) |
                    (intB & 0xFFu);

                palette[offset++] =
                    setGamma(rgb, ClassicGamma);
            }
        }

        return palette;
    }();

    return Palette;
}

TerrainCornerShades makeCornerShades(
    int southwest,
    int southeast,
    int northeast,
    int northwest
) {
    return TerrainCornerShades{
        southwest,
        southeast,
        northeast,
        northwest
    };
}

}

int ClassicTerrainAppearanceBuilder::vertexLight(
    std::size_t plane,
    int x,
    int y,
    const TerrainTileSampler& sample
) {
    const eld::map::MapTile* west = sample(plane, x - 1, y);
    const eld::map::MapTile* east = sample(plane, x + 1, y);
    const eld::map::MapTile* south = sample(plane, x, y - 1);
    const eld::map::MapTile* north = sample(plane, x, y + 1);

    if (
        west == nullptr || east == nullptr ||
        south == nullptr || north == nullptr
    ) {
        return 96;
    }

    const int dx = east->height - west->height;
    const int dy = north->height - south->height;
    const int length = static_cast<int>(
        std::sqrt(
            static_cast<double>(dx) * dx +
            static_cast<double>(dy) * dy +
            65536.0
        )
    );

    if (length <= 0) {
        return 96;
    }

    const int normalX = dx * 256 / length;
    const int normalY = 65536 / length;
    const int normalZ = dy * 256 / length;

    constexpr int Ambient = 96;
    constexpr int Attenuation = 768;
    constexpr int LightX = -50;
    constexpr int LightY = -10;
    constexpr int LightZ = -50;

    const int lightVectorLength = static_cast<int>(
        std::sqrt(
            static_cast<double>(
                LightX * LightX +
                LightY * LightY +
                LightZ * LightZ
            )
        )
    );
    const int lightMagnitude =
        Attenuation * lightVectorLength >> 8;

    if (lightMagnitude == 0) {
        return Ambient;
    }

    return
        Ambient +
        (
            LightX * normalX +
            LightY * normalY +
            LightZ * normalZ
        ) /
        lightMagnitude;
}

SceneTileAppearance ClassicTerrainAppearanceBuilder::build(
    std::size_t plane,
    int tileX,
    int tileY,
    const TerrainTileSampler& sample,
    const eld::definition::FloorRepository& floors
) const {
    SceneTileAppearance result;

    const eld::map::MapTile* tile =
        sample(plane, tileX, tileY);
    if (tile == nullptr) {
        return result;
    }

    const int lightSW = vertexLight(plane, tileX, tileY, sample);
    const int lightSE = vertexLight(plane, tileX + 1, tileY, sample);
    const int lightNE = vertexLight(plane, tileX + 1, tileY + 1, sample);
    const int lightNW = vertexLight(plane, tileX, tileY + 1, sample);

    int chromaSum = 0;
    int saturationSum = 0;
    int lightnessSum = 0;
    int luminanceSum = 0;
    int count = 0;

    // The original rolling ±5 accumulator has already added x/z + 5 and
    // removed x/z - 5 when a tile is shaded. The effective interior window
    // is therefore [-4, +5] on each axis (10 x 10 samples).
    for (int dx = -4; dx <= 5; ++dx) {
        for (int dy = -4; dy <= 5; ++dy) {
            const eld::map::MapTile* neighbor =
                sample(plane, tileX + dx, tileY + dy);
            if (neighbor == nullptr || neighbor->underlayId == 0) {
                continue;
            }

            const std::uint32_t rgb =
                floorRgb(neighbor->underlayId, floors);
            const FloorHsl hsl = rgbToClassicHsl(rgb);

            chromaSum += hsl.chroma;
            saturationSum += hsl.saturation;
            lightnessSum += hsl.lightness;
            luminanceSum += hsl.luminance;
            ++count;
        }
    }

    int underlayBase = -1;
    if (
        tile->underlayId != 0 &&
        count > 0 &&
        luminanceSum > 0
    ) {
        const int hue = chromaSum * 256 / luminanceSum;
        const int saturation = saturationSum / count;
        const int lightness = lightnessSum / count;
        underlayBase = hsl24to16(hue, saturation, lightness);
        result.underlayVisible = true;
    }

    result.shades.underlay = makeCornerShades(
        mulHsl(underlayBase, lightSW),
        mulHsl(underlayBase, lightSE),
        mulHsl(underlayBase, lightNE),
        mulHsl(underlayBase, lightNW)
    );

    if (tile->overlayId == 0) {
        return result;
    }

    const auto* overlay = floors.find(
        static_cast<std::uint16_t>(tile->overlayId - 1u)
    );

    if (
        overlay != nullptr &&
        overlay->rgb.has_value() &&
        *overlay->rgb == 0xFF00FFu
    ) {
        // Classic sentinel: this floor is intentionally not drawn.
        result.overlayVisible = false;
        return result;
    }

    result.overlayVisible = true;

    if (
        overlay != nullptr &&
        overlay->textureId.has_value()
    ) {
        result.textureId = overlay->textureId;
        result.shades.overlay = makeCornerShades(
            adjustLightness(-1, lightSW),
            adjustLightness(-1, lightSE),
            adjustLightness(-1, lightNE),
            adjustLightness(-1, lightNW)
        );
        return result;
    }

    const std::uint32_t overlayRgb =
        floorRgb(tile->overlayId, floors);
    const FloorHsl overlayHsl = rgbToClassicHsl(overlayRgb);
    const int packedOverlay = hsl24to16(
        overlayHsl.hue,
        overlayHsl.saturation,
        overlayHsl.lightness
    );

    result.shades.overlay = makeCornerShades(
        adjustLightness(packedOverlay, lightSW),
        adjustLightness(packedOverlay, lightSE),
        adjustLightness(packedOverlay, lightNE),
        adjustLightness(packedOverlay, lightNW)
    );

    return result;
}

std::uint32_t ClassicTerrainAppearanceBuilder::paletteRgb(
    int paletteIndex
) {
    if (
        paletteIndex < 0 ||
        paletteIndex >= static_cast<int>(classicPalette().size())
    ) {
        return 0;
    }

    return classicPalette()[static_cast<std::size_t>(paletteIndex)];
}

std::uint32_t ClassicTerrainAppearanceBuilder::textureModulationRgb(
    int textureShade
) {
    const int light = std::clamp(127 - textureShade, 0, 127);
    const double factor = std::clamp(
        static_cast<double>(light) / 96.0,
        0.35,
        1.0
    );

    const std::uint32_t channel =
        static_cast<std::uint32_t>(
            std::clamp(
                static_cast<int>(255.0 * factor),
                0,
                255
            )
        );

    return (channel << 16) | (channel << 8) | channel;
}

}
