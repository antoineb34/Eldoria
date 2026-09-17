#include "ClassicTerrainLighting.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

#include "render/map/ClassicTerrainAppearance.h"

namespace eld::graphics::terrain
{

namespace
{

struct FloorHsl
{
    int hue = 0;
    int saturation = 0;
    int lightness = 0;
    int chroma = 0;
    int luminance = 1;
};


std::uint32_t fallbackFloorRgb(
    eld::world::FloorId id
)
{
    // World FloorId is already the classic raw ID - 1.
    const std::uint32_t rawId =
        static_cast<std::uint32_t>(id) + 1u;

    const std::uint32_t value =
        rawId * 2654435761u;

    const std::uint32_t r =
        64u + ((value >> 16) & 0x7Fu);

    const std::uint32_t g =
        64u + ((value >> 8) & 0x7Fu);

    const std::uint32_t b =
        64u + (value & 0x7Fu);

    return
        (r << 16) |
        (g << 8) |
        b;
}


std::uint32_t floorRgb(
    eld::world::FloorId id,
    const eld::floor::FloorLoader& floors
)
{
    const auto floor =
        floors.find(id);

    if (!floor.has_value())
    {
        return fallbackFloorRgb(id);
    }

    if (floor->rgb.has_value())
    {
        return *floor->rgb;
    }

    if (floor->secondaryRgb.has_value())
    {
        return *floor->secondaryRgb;
    }

    return fallbackFloorRgb(id);
}


FloorHsl rgbToClassicHsl(
    std::uint32_t rgb
)
{
    const double red =
        static_cast<double>(
            (rgb >> 16) & 0xFFu
        ) / 256.0;

    const double green =
        static_cast<double>(
            (rgb >> 8) & 0xFFu
        ) / 256.0;

    const double blue =
        static_cast<double>(
            rgb & 0xFFu
        ) / 256.0;

    const double minimum =
        std::min({
            red,
            green,
            blue
        });

    const double maximum =
        std::max({
            red,
            green,
            blue
        });

    double h = 0.0;
    double s = 0.0;

    const double l =
        (minimum + maximum) /
        2.0;

    if (minimum != maximum)
    {
        if (l < 0.5)
        {
            s =
                (maximum - minimum) /
                (maximum + minimum);
        }
        else
        {
            s =
                (maximum - minimum) /
                (
                    2.0 -
                    maximum -
                    minimum
                );
        }

        if (red == maximum)
        {
            h =
                (green - blue) /
                (maximum - minimum);
        }
        else if (green == maximum)
        {
            h =
                (
                    blue - red
                ) /
                (
                    maximum - minimum
                ) +
                2.0;
        }
        else
        {
            h =
                (
                    red - green
                ) /
                (
                    maximum - minimum
                ) +
                4.0;
        }
    }

    h /= 6.0;

    FloorHsl result;

    result.hue =
        static_cast<int>(
            h * 256.0
        );

    result.saturation =
        std::clamp(
            static_cast<int>(
                s * 256.0
            ),
            0,
            255
        );

    result.lightness =
        std::clamp(
            static_cast<int>(
                l * 256.0
            ),
            0,
            255
        );

    result.luminance =
        l > 0.5
            ? static_cast<int>(
                  (1.0 - l) *
                  s *
                  512.0
              )
            : static_cast<int>(
                  l *
                  s *
                  512.0
              );

    result.luminance =
        std::max(
            result.luminance,
            1
        );

    result.chroma =
        static_cast<int>(
            h *
            static_cast<double>(
                result.luminance
            )
        );

    return result;
}


int hsl24to16(
    int hue,
    int saturation,
    int lightness
)
{
    if (lightness > 179)
    {
        saturation /= 2;
    }

    if (lightness > 192)
    {
        saturation /= 2;
    }

    if (lightness > 217)
    {
        saturation /= 2;
    }

    if (lightness > 243)
    {
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
)
{
    constexpr int InvisibleShade =
        12345678;

    if (hsl == -1)
    {
        return InvisibleShade;
    }

    const int low =
        static_cast<int>(
            static_cast<std::uint32_t>(
                hsl
            ) &
            0x7Fu
        );

    const int high =
        static_cast<int>(
            static_cast<std::uint32_t>(
                hsl
            ) &
            0xFF80u
        );

    lightness =
        lightness *
        low /
        128;

    lightness =
        std::clamp(
            lightness,
            2,
            126
        );

    return
        high +
        lightness;
}


int adjustLightness(
    int hsl,
    int scalar
)
{
    constexpr int InvisibleShade =
        12345678;

    if (hsl == -2)
    {
        return InvisibleShade;
    }

    if (hsl == -1)
    {
        scalar =
            std::clamp(
                scalar,
                0,
                127
            );

        return
            127 -
            scalar;
    }

    const int low =
        static_cast<int>(
            static_cast<std::uint32_t>(
                hsl
            ) &
            0x7Fu
        );

    const int high =
        static_cast<int>(
            static_cast<std::uint32_t>(
                hsl
            ) &
            0xFF80u
        );

    scalar =
        scalar *
        low /
        128;

    scalar =
        std::clamp(
            scalar,
            2,
            126
        );

    return
        high +
        scalar;
}


const eld::world::Tile* tileAt(
    const eld::world::Terrain& terrain,
    std::size_t plane,
    int x,
    int y
)
{
    if (
        plane >=
            terrain.sourcePlaneCount() ||
        x < 0 ||
        y < 0 ||
        x >=
            static_cast<int>(
                terrain.width()
            ) ||
        y >=
            static_cast<int>(
                terrain.height()
            )
    )
    {
        return nullptr;
    }

    const auto position =
        terrain.layerPosition(
            plane,
            static_cast<std::size_t>(x),
            static_cast<std::size_t>(y)
        );

    return &terrain.tile(position);
}


// Reconstruct the classic cache height integer from World's
// scaled height.
//
// RegionBuilder uses:
//
//     worldHeight = cacheHeight * (-1 / 128)
//
// so:
//
//     cacheHeight = worldHeight * -128
//
std::optional<int> cacheHeightAtCorner(
    const eld::world::Terrain& terrain,
    std::size_t plane,
    int x,
    int y
)
{
    if (
        plane >=
            terrain.sourcePlaneCount() ||
        x < 0 ||
        y < 0 ||
        x >
            static_cast<int>(
                terrain.width()
            ) ||
        y >
            static_cast<int>(
                terrain.height()
            ) ||
        terrain.width() == 0 ||
        terrain.height() == 0
    )
    {
        return std::nullopt;
    }

    const std::size_t tileX =
        std::min(
            static_cast<std::size_t>(x),
            terrain.width() - 1u
        );

    const std::size_t tileY =
        std::min(
            static_cast<std::size_t>(y),
            terrain.height() - 1u
        );

    const auto position =
        terrain.layerPosition(
            plane,
            tileX,
            tileY
        );

    const auto heights =
        terrain.cornerHeights(
            position
        );

    float height = 0.0f;

    const int tx =
        static_cast<int>(tileX);

    const int ty =
        static_cast<int>(tileY);

    if (
        x == tx &&
        y == ty
    )
    {
        height =
            heights.southwest;
    }
    else if (
        x == tx + 1 &&
        y == ty
    )
    {
        height =
            heights.southeast;
    }
    else if (
        x == tx + 1 &&
        y == ty + 1
    )
    {
        height =
            heights.northeast;
    }
    else if (
        x == tx &&
        y == ty + 1
    )
    {
        height =
            heights.northwest;
    }
    else
    {
        return std::nullopt;
    }

    return static_cast<int>(
        std::lround(
            -height *
            128.0f
        )
    );
}


int vertexLight(
    const eld::world::Terrain& terrain,
    std::size_t plane,
    int x,
    int y
)
{
    const auto west =
        cacheHeightAtCorner(
            terrain,
            plane,
            x - 1,
            y
        );

    const auto east =
        cacheHeightAtCorner(
            terrain,
            plane,
            x + 1,
            y
        );

    const auto south =
        cacheHeightAtCorner(
            terrain,
            plane,
            x,
            y - 1
        );

    const auto north =
        cacheHeightAtCorner(
            terrain,
            plane,
            x,
            y + 1
        );

    if (
        !west.has_value() ||
        !east.has_value() ||
        !south.has_value() ||
        !north.has_value()
    )
    {
        return 96;
    }

    const int dx =
        *east -
        *west;

    const int dy =
        *north -
        *south;

    const int length =
        static_cast<int>(
            std::sqrt(
                static_cast<double>(dx) *
                    dx +
                static_cast<double>(dy) *
                    dy +
                65536.0
            )
        );

    if (length <= 0)
    {
        return 96;
    }

    const int normalX =
        dx *
        256 /
        length;

    const int normalY =
        65536 /
        length;

    const int normalZ =
        dy *
        256 /
        length;

    constexpr int Ambient =
        96;

    constexpr int Attenuation =
        768;

    constexpr int LightX =
        -50;

    constexpr int LightY =
        -10;

    constexpr int LightZ =
        -50;

    const int lightVectorLength =
        static_cast<int>(
            std::sqrt(
                static_cast<double>(
                    LightX * LightX +
                    LightY * LightY +
                    LightZ * LightZ
                )
            )
        );

    const int lightMagnitude =
        Attenuation *
        lightVectorLength >>
        8;

    if (lightMagnitude == 0)
    {
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


int midpoint(
    int a,
    int b
)
{
    return
        (a + b) /
        2;
}


eld::math::Vec4 rgbToVec4(
    std::uint32_t rgb
)
{
    constexpr float Scale =
        1.0f /
        255.0f;

    return {
        static_cast<float>(
            (rgb >> 16) &
            0xFFu
        ) *
            Scale,

        static_cast<float>(
            (rgb >> 8) &
            0xFFu
        ) *
            Scale,

        static_cast<float>(
            rgb &
            0xFFu
        ) *
            Scale,

        1.0f
    };
}

}


ClassicTileLighting buildClassicTerrainLighting(
    const eld::world::Terrain& terrain,
    const eld::world::TerrainLayerPosition& position,
    const eld::floor::FloorLoader& floors
)
{
    ClassicTileLighting result;

    const std::size_t plane =
        static_cast<std::size_t>(
            position.sourcePlane
        );

    const auto& origin =
        terrain.origin();

    const int tileX =
        position.x -
        origin.x;

    const int tileY =
        position.y -
        origin.y;

    const auto& tile =
        terrain.tile(position);

    const int lightSW =
        vertexLight(
            terrain,
            plane,
            tileX,
            tileY
        );

    const int lightSE =
        vertexLight(
            terrain,
            plane,
            tileX + 1,
            tileY
        );

    const int lightNE =
        vertexLight(
            terrain,
            plane,
            tileX + 1,
            tileY + 1
        );

    const int lightNW =
        vertexLight(
            terrain,
            plane,
            tileX,
            tileY + 1
        );


    // --------------------------------------------------------
    // UNDERLAY
    //
    // Classic client smooths surrounding floor HSL before
    // applying corner lighting.
    //
    // Effective window is [-4, +5] on each axis.
    // --------------------------------------------------------

    if (
        tile.surface.underlay
            .has_value()
    )
    {
        int chromaSum = 0;
        int saturationSum = 0;
        int lightnessSum = 0;
        int luminanceSum = 0;
        int count = 0;

        for (
            int dx = -4;
            dx <= 5;
            ++dx
        )
        {
            for (
                int dy = -4;
                dy <= 5;
                ++dy
            )
            {
                const auto* neighbor =
                    tileAt(
                        terrain,
                        plane,
                        tileX + dx,
                        tileY + dy
                    );

                if (
                    neighbor == nullptr ||
                    !neighbor
                         ->surface
                         .underlay
                         .has_value()
                )
                {
                    continue;
                }

                const auto hsl =
                    rgbToClassicHsl(
                        floorRgb(
                            *neighbor
                                 ->surface
                                 .underlay,
                            floors
                        )
                    );

                chromaSum +=
                    hsl.chroma;

                saturationSum +=
                    hsl.saturation;

                lightnessSum +=
                    hsl.lightness;

                luminanceSum +=
                    hsl.luminance;

                ++count;
            }
        }

        if (
            count > 0 &&
            luminanceSum > 0
        )
        {
            const int hue =
                chromaSum *
                256 /
                luminanceSum;

            const int saturation =
                saturationSum /
                count;

            const int lightness =
                lightnessSum /
                count;

            const int underlayBase =
                hsl24to16(
                    hue,
                    saturation,
                    lightness
                );

            result.underlay.visible =
                true;

            result.underlay.shades = {
                mulHsl(
                    underlayBase,
                    lightSW
                ),

                mulHsl(
                    underlayBase,
                    lightSE
                ),

                mulHsl(
                    underlayBase,
                    lightNE
                ),

                mulHsl(
                    underlayBase,
                    lightNW
                )
            };
        }
    }


    // --------------------------------------------------------
    // OVERLAY
    // --------------------------------------------------------

    if (
        !tile.surface.overlay
             .has_value()
    )
    {
        return result;
    }

    const auto overlayId =
        *tile.surface.overlay;

    const auto overlay =
        floors.find(
            overlayId
        );

    if (
        overlay.has_value() &&
        overlay->rgb.has_value() &&
        *overlay->rgb ==
            0xFF00FFu
    )
    {
        // Classic sentinel:
        // intentionally invisible floor.
        return result;
    }

    result.overlay.visible =
        true;

    if (
        overlay.has_value() &&
        overlay
            ->textureId
            .has_value()
    )
    {
        result.overlay.shades = {
            adjustLightness(
                -1,
                lightSW
            ),

            adjustLightness(
                -1,
                lightSE
            ),

            adjustLightness(
                -1,
                lightNE
            ),

            adjustLightness(
                -1,
                lightNW
            )
        };

        return result;
    }

    const std::uint32_t overlayRgb =
        floorRgb(
            overlayId,
            floors
        );

    const auto overlayHsl =
        rgbToClassicHsl(
            overlayRgb
        );

    const int packedOverlay =
        hsl24to16(
            overlayHsl.hue,
            overlayHsl.saturation,
            overlayHsl.lightness
        );

    result.overlay.shades = {
        adjustLightness(
            packedOverlay,
            lightSW
        ),

        adjustLightness(
            packedOverlay,
            lightSE
        ),

        adjustLightness(
            packedOverlay,
            lightNE
        ),

        adjustLightness(
            packedOverlay,
            lightNW
        )
    };

    return result;
}


int classicShadeForPoint(
    int pointType,
    const ClassicCornerShades& shades
)
{
    switch (pointType)
    {
    case 1:
    case 13:
        return shades.southwest;

    case 2:
    case 9:
        return midpoint(
            shades.southwest,
            shades.southeast
        );

    case 3:
    case 14:
        return shades.southeast;

    case 4:
    case 10:
        return midpoint(
            shades.southeast,
            shades.northeast
        );

    case 5:
    case 15:
        return shades.northeast;

    case 6:
    case 11:
        return midpoint(
            shades.northeast,
            shades.northwest
        );

    case 7:
    case 16:
        return shades.northwest;

    case 8:
    case 12:
        return midpoint(
            shades.northwest,
            shades.southwest
        );

    default:
        return shades.southwest;
    }
}


eld::math::Vec4 classicTerrainColor(
    int shade,
    bool textured
)
{
    const std::uint32_t rgb =
        textured
            ? eld::render::map::
                  ClassicTerrainAppearanceBuilder::
                  textureModulationRgb(
                      shade
                  )
            : eld::render::map::
                  ClassicTerrainAppearanceBuilder::
                  paletteRgb(
                      shade
                  );

    return rgbToVec4(rgb);
}

}
