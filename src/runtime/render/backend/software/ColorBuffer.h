#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::render {

struct ColorPixel {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 255;
};

class ColorBuffer {
public:
    void resize(
        std::uint32_t width,
        std::uint32_t height
    ) {
        width_ = width;
        height_ = height;

        pixels_.resize(
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height)
        );
    }

    void clear(
        ColorPixel color = {}
    ) {
        std::fill(
            pixels_.begin(),
            pixels_.end(),
            color
        );
    }

    ColorPixel& at(
        std::uint32_t x,
        std::uint32_t y
    ) {
        return pixels_.at(
            static_cast<std::size_t>(y) * width_ + x
        );
    }

    const ColorPixel& at(
        std::uint32_t x,
        std::uint32_t y
    ) const {
        return pixels_.at(
            static_cast<std::size_t>(y) * width_ + x
        );
    }

    const ColorPixel* data() const {
        return pixels_.data();
    }

    std::uint32_t width() const {
        return width_;
    }

    std::uint32_t height() const {
        return height_;
    }

private:
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;

    std::vector<ColorPixel> pixels_;
};

}
