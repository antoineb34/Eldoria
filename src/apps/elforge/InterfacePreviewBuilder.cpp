#include "InterfacePreviewBuilder.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_set>

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

void drawInventory(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget
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
}

void drawWidget(
    eld::image::Image& image,
    int x,
    int y,
    const eld::interface::InterfaceDefinition& widget,
    const eld::interface::InterfaceRepository& repository,
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
            widget
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
                path
            );
        }
    }

    path.erase(widget.id);
}

}

eld::image::Image InterfacePreviewBuilder::build(
    const eld::interface::InterfaceDefinition& root,
    const eld::interface::InterfaceRepository& repository
) const {
    eld::image::Image image;

    image.width = CanvasWidth;
    image.height = CanvasHeight;

    image.pixels.assign(
        static_cast<std::size_t>(CanvasWidth) *
        CanvasHeight,
        makePixel(0x17191C)
    );

    std::unordered_set<std::uint16_t> path;

    drawWidget(
        image,
        0,
        0,
        root,
        repository,
        path
    );

    return image;
}

}
