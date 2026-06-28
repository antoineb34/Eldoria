#include "InterfacePreviewBuilder.h"
#include "ModelThumbnailRenderer.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace eld::elforge {

namespace {

constexpr int CanvasWidth = 765;
constexpr int CanvasHeight = 503;

eld::image::RgbaPixel makePixel(
    std::uint32_t rgb,
    std::uint8_t alpha = 255
) {
    return {
        static_cast<std::uint8_t>(
            (rgb >> 16) & 0xFF
        ),
        static_cast<std::uint8_t>(
            (rgb >> 8) & 0xFF
        ),
        static_cast<std::uint8_t>(
            rgb & 0xFF
        ),
        alpha
    };
}

void setPixel(
    eld::image::Image& image,
    int x,
    int y,
    eld::image::RgbaPixel color
) {
    if (
        x < 0 ||
        y < 0 ||
        x >= image.width ||
        y >= image.height
    ) {
        return;
    }

    image.pixels[
        static_cast<std::size_t>(y) *
        image.width +
        static_cast<std::size_t>(x)
    ] = color;
}

void fillRectangle(
    eld::image::Image& image,
    int x,
    int y,
    int width,
    int height,
    eld::image::RgbaPixel color
) {
    for (
        int drawY = std::max(0, y);
        drawY < std::min<int>(image.height, y + height);
        ++drawY
    ) {
        for (
            int drawX = std::max(0, x);
            drawX < std::min<int>(image.width, x + width);
            ++drawX
        ) {
            setPixel(
                image,
                drawX,
                drawY,
                color
            );
        }
    }
}

void drawOutline(
    eld::image::Image& image,
    int x,
    int y,
    int width,
    int height,
    eld::image::RgbaPixel color
) {
    if (width <= 0 || height <= 0) {
        return;
    }

    for (int drawX = x; drawX < x + width; ++drawX) {
        setPixel(image, drawX, y, color);
        setPixel(image, drawX, y + height - 1, color);
    }

    for (int drawY = y; drawY < y + height; ++drawY) {
        setPixel(image, x, drawY, color);
        setPixel(image, x + width - 1, drawY, color);
    }
}

int getDisplayWidth(
    const eld::interface::InterfaceDefinition& widget
) {
    if (
        widget.type == 2 ||
        widget.type == 7
    ) {
        return
            static_cast<int>(widget.width) * 32 +
            std::max(
                0,
                static_cast<int>(widget.width) - 1
            ) *
            (
                widget.type == 2
                    ? widget.inventoryPaddingX
                    : widget.itemPaddingX
            );
    }

    return widget.width;
}

int getDisplayHeight(
    const eld::interface::InterfaceDefinition& widget
) {
    if (
        widget.type == 2 ||
        widget.type == 7
    ) {
        return
            static_cast<int>(widget.height) * 32 +
            std::max(
                0,
                static_cast<int>(widget.height) - 1
            ) *
            (
                widget.type == 2
                    ? widget.inventoryPaddingY
                    : widget.itemPaddingY
            );
    }

    return widget.height;
}

std::uint32_t getTypeColor(
    std::uint8_t type
) {
    switch (type) {
        case 0:
            return 0x73829A;
        case 2:
        case 7:
            return 0xB58A43;
        case 3:
            return 0x5D9B63;
        case 4:
        case 8:
            return 0xD7C56D;
        case 5:
            return 0x5C91C9;
        case 6:
            return 0xA66CC2;
        default:
            return 0x8A8A8A;
    }
}


std::optional<std::pair<std::string, std::uint16_t>>
parseSpriteReference(
    const std::string& reference
) {
    const std::size_t comma =
        reference.rfind(',');

    if (comma == std::string::npos) {
        return std::nullopt;
    }

    std::string group =
        reference.substr(0, comma);

    while (
        !group.empty() &&
        group.back() == ' '
    ) {
        group.pop_back();
    }

    if (group.empty()) {
        return std::nullopt;
    }

    if (!group.ends_with(".dat")) {
        group += ".dat";
    }

    const std::string frameText =
        reference.substr(comma + 1);

    unsigned int frame = 0;

    const auto result =
        std::from_chars(
            frameText.data(),
            frameText.data() + frameText.size(),
            frame
        );

    if (
        result.ec != std::errc{} ||
        frame >
            std::numeric_limits<std::uint16_t>::max()
    ) {
        return std::nullopt;
    }

    return std::pair{
        std::move(group),
        static_cast<std::uint16_t>(frame)
    };
}

void blendPixel(
    eld::image::Image& destination,
    int x,
    int y,
    const eld::image::RgbaPixel& source
) {
    if (
        source.alpha == 0 ||
        x < 0 ||
        y < 0 ||
        x >= destination.width ||
        y >= destination.height
    ) {
        return;
    }

    eld::image::RgbaPixel& target =
        destination.pixels[
            static_cast<std::size_t>(y) *
            destination.width +
            static_cast<std::size_t>(x)
        ];

    const unsigned int alpha =
        source.alpha;

    const unsigned int inverse =
        255 - alpha;

    target.red =
        static_cast<std::uint8_t>(
            (
                source.red * alpha +
                target.red * inverse
            ) / 255
        );

    target.green =
        static_cast<std::uint8_t>(
            (
                source.green * alpha +
                target.green * inverse
            ) / 255
        );

    target.blue =
        static_cast<std::uint8_t>(
            (
                source.blue * alpha +
                target.blue * inverse
            ) / 255
        );

    target.alpha = 255;
}

void blitImage(
    eld::image::Image& destination,
    int x,
    int y,
    const eld::image::Image& source
) {
    for (
        int sourceY = 0;
        sourceY < source.height;
        ++sourceY
    ) {
        for (
            int sourceX = 0;
            sourceX < source.width;
            ++sourceX
        ) {
            blendPixel(
                destination,
                x + sourceX,
                y + sourceY,
                source.pixels[
                    static_cast<std::size_t>(sourceY) *
                    source.width +
                    static_cast<std::size_t>(sourceX)
                ]
            );
        }
    }
}

bool drawSpriteReference(
    eld::image::Image& image,
    int x,
    int y,
    const std::string& reference,
    const eld::sprite::SpriteRepository& repository
) {
    const auto parsed =
        parseSpriteReference(reference);

    if (!parsed.has_value()) {
        return false;
    }

    const auto& [group, frameId] =
        *parsed;

    const std::optional<eld::sprite::Sprite> sprite =
        repository.find(
            group,
            frameId
        );

    if (!sprite.has_value()) {
        return false;
    }

    blitImage(
        image,
        x,
        y,
        sprite->image
    );

    return true;
}

using FontSet =
    std::array<std::optional<eld::font::Font>, 4>;

const eld::font::Glyph* findGlyph(
    const eld::font::Font& font,
    char character
) {
    const std::uint16_t code =
        static_cast<std::uint8_t>(character);

    for (const eld::font::Glyph& glyph : font.glyphs) {
        if (glyph.character == code) {
            return &glyph;
        }
    }

    return nullptr;
}

std::string stripColorTags(
    std::string_view text
) {
    std::string result;

    for (std::size_t index = 0; index < text.size();) {
        if (
            index + 4 < text.size() &&
            text[index] == '@' &&
            text[index + 4] == '@'
        ) {
            index += 5;
            continue;
        }

        result.push_back(text[index]);
        ++index;
    }

    return result;
}

int measureText(
    const eld::font::Font& font,
    std::string_view text
) {
    int width = 0;
    const std::string cleaned =
        stripColorTags(text);

    for (const char character : cleaned) {
        if (character == '\n') {
            break;
        }

        const eld::font::Glyph* glyph =
            findGlyph(font, character);

        width += glyph != nullptr
            ? static_cast<int>(glyph->advance)
            : static_cast<int>(font.lineHeight / 2);
    }

    return width;
}

void drawGlyph(
    eld::image::Image& image,
    int x,
    int y,
    const eld::font::Glyph& glyph,
    eld::image::RgbaPixel color
) {
    const std::size_t expected =
        static_cast<std::size_t>(glyph.width) *
        glyph.height;

    if (glyph.alpha.size() != expected) {
        return;
    }

    for (std::uint16_t glyphY = 0; glyphY < glyph.height; ++glyphY) {
        for (std::uint16_t glyphX = 0; glyphX < glyph.width; ++glyphX) {
            const std::uint8_t alpha =
                glyph.alpha[
                    static_cast<std::size_t>(glyphY) *
                    glyph.width +
                    glyphX
                ];

            if (alpha == 0) {
                continue;
            }

            eld::image::RgbaPixel pixel = color;
            pixel.alpha =
                static_cast<std::uint8_t>(
                    static_cast<unsigned int>(pixel.alpha) *
                    alpha /
                    255
                );

            setPixel(
                image,
                x + glyph.offsetX + glyphX,
                y + glyph.offsetY + glyphY,
                pixel
            );
        }
    }
}

void drawText(
    eld::image::Image& image,
    int x,
    int y,
    std::string_view text,
    const eld::font::Font& font,
    eld::image::RgbaPixel color,
    bool centered,
    int width
) {
    const std::string cleaned =
        stripColorTags(text);

    int cursorX =
        centered
            ? x + (width - measureText(font, cleaned)) / 2
            : x;

    int cursorY = y;

    for (const char character : cleaned) {
        if (character == '\n') {
            cursorX =
                centered
                    ? x + (width - measureText(font, cleaned)) / 2
                    : x;

            cursorY += font.lineHeight;
            continue;
        }

        const eld::font::Glyph* glyph =
            findGlyph(font, character);

        if (glyph == nullptr) {
            cursorX += font.lineHeight / 2;
            continue;
        }

        drawGlyph(
            image,
            cursorX,
            cursorY,
            *glyph,
            color
        );

        cursorX += glyph->advance;
    }
}

void drawInterfaceText(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget,
    const FontSet& fonts
) {
    if (widget.text.empty()) {
        return;
    }

    const std::size_t fontIndex =
        widget.fontId < fonts.size()
            ? widget.fontId
            : 0;

    if (!fonts[fontIndex].has_value()) {
        return;
    }

    const eld::font::Font& font =
        *fonts[fontIndex];

    if (widget.textShadow) {
        drawText(
            image,
            x + 1,
            y + 1,
            widget.text,
            font,
            eld::image::RgbaPixel{0, 0, 0, 180},
            widget.centeredText,
            widget.width
        );
    }

    drawText(
        image,
        x,
        y,
        widget.text,
        font,
        makePixel(widget.color != 0 ? widget.color : 0xFFFF00),
        widget.centeredText,
        widget.width
    );
}

void drawModelThumbnail(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget,
    eld::graphics::GraphicsResources& graphicsResources
) {
    if (!widget.modelId.has_value()) {
        return;
    }

    try {
        const eld::graphics::ModelHandle handle =
            graphicsResources.resolveModel(
                *widget.modelId
            );

        const ModelThumbnailRenderer renderer;

        eld::image::Image thumbnail =
            renderer.render(
                handle,
                graphicsResources,
                std::max<std::uint16_t>(1, widget.width),
                std::max<std::uint16_t>(1, widget.height),
                widget.modelZoom,
                widget.modelRotationX,
                widget.modelRotationY
            );

        blitImage(
            image,
            x,
            y,
            thumbnail
        );
    }
    catch (const std::exception&) {
    }
}

void drawInventory(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget,
    const eld::sprite::SpriteRepository& spriteRepository
) {
    const int paddingX =
        widget.type == 2
            ? widget.inventoryPaddingX
            : widget.itemPaddingX;

    const int paddingY =
        widget.type == 2
            ? widget.inventoryPaddingY
            : widget.itemPaddingY;

    for (
        int row = 0;
        row < widget.height;
        ++row
    ) {
        for (
            int column = 0;
            column < widget.width;
            ++column
        ) {
            const int slotX =
                x + column * (32 + paddingX);

            const int slotY =
                y + row * (32 + paddingY);

            fillRectangle(
                image,
                slotX,
                slotY,
                32,
                32,
                makePixel(0x39362F)
            );

            drawOutline(
                image,
                slotX,
                slotY,
                32,
                32,
                makePixel(0xB58A43)
            );
        }
    }

    for (
        const eld::interface::InterfaceSpriteSlot& slot :
        widget.inventorySprites
    ) {
        drawSpriteReference(
            image,
            x + slot.x,
            y + slot.y,
            slot.sprite,
            spriteRepository
        );
    }
}

void drawWidget(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget,
    const eld::interface::InterfaceRepository& repository,
    const eld::sprite::SpriteRepository& spriteRepository,
    const FontSet& fonts,
    eld::graphics::GraphicsResources& graphicsResources,
    std::unordered_set<std::uint16_t>& path
) {
    if (!path.insert(widget.id).second) {
        return;
    }

    const int width =
        std::max(1, getDisplayWidth(widget));

    const int height =
        std::max(1, getDisplayHeight(widget));

    if (
        widget.type == 2 ||
        widget.type == 7
    ) {
        drawInventory(
            image,
            x,
            y,
            widget,
            spriteRepository
        );
    }
    else if (
        widget.type == 5 &&
        drawSpriteReference(
            image,
            x,
            y,
            widget.sprite,
            spriteRepository
        )
    ) {
    }
    else if (
        widget.type == 4 ||
        widget.type == 8
    ) {
        drawInterfaceText(
            image,
            x,
            y,
            widget,
            fonts
        );
    }
    else if (widget.type == 6) {
        drawModelThumbnail(
            image,
            x,
            y,
            widget,
            graphicsResources
        );
    }
    else if (widget.type == 3) {
        const std::uint32_t color =
            widget.color != 0
                ? widget.color
                : getTypeColor(widget.type);

        if (widget.filled) {
            fillRectangle(
                image,
                x,
                y,
                width,
                height,
                makePixel(color)
            );
        }

        drawOutline(
            image,
            x,
            y,
            width,
            height,
            makePixel(color)
        );
    }
    else {
        fillRectangle(
            image,
            x,
            y,
            width,
            height,
            makePixel(
                getTypeColor(widget.type),
                widget.type == 0 ? 35 : 150
            )
        );

        drawOutline(
            image,
            x,
            y,
            width,
            height,
            makePixel(getTypeColor(widget.type))
        );
    }

    for (
        const eld::interface::InterfaceChild& child :
        widget.children
    ) {
        const auto* childWidget =
            repository.find(child.id);

        if (childWidget != nullptr) {
            drawWidget(
                image,
                x + child.x,
                y + child.y,
                *childWidget,
                repository,
                spriteRepository,
                fonts,
                graphicsResources,
                path
            );
        }
    }

    path.erase(widget.id);
}

}

eld::image::Image InterfacePreviewBuilder::build(
    const eld::interface::InterfaceDefinition& root,
    const eld::interface::InterfaceRepository& repository,
    const eld::sprite::SpriteRepository& spriteRepository,
    const eld::font::FontRepository& fontRepository,
    eld::graphics::GraphicsResources& graphicsResources
) const {
    eld::image::Image image;

    image.width = CanvasWidth;
    image.height = CanvasHeight;

    image.pixels.assign(
        static_cast<std::size_t>(CanvasWidth) *
        CanvasHeight,
        makePixel(0x17191C)
    );

    const FontSet fonts{
        fontRepository.find("p11_full.dat"),
        fontRepository.find("p12_full.dat"),
        fontRepository.find("b12_full.dat"),
        fontRepository.find("q8_full.dat")
    };

    std::unordered_set<std::uint16_t> path;

    drawWidget(
        image,
        0,
        0,
        root,
        repository,
        spriteRepository,
        fonts,
        graphicsResources,
        path
    );

    return image;
}

}
