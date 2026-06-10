#pragma once

#include <cstdint>
#include <vector>

namespace eld::render {

struct ColorPixel {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

class ColorBuffer {
public:
    void resize(
        int width,
        int height
    ) {
        width_ = width;
        height_ = height;
        pixels_.resize(width * height);
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
        int x,
        int y
    ) {
        return pixels_[y * width_ + x];
    }

    const ColorPixel& at(
        int x,
        int y
    ) const {
        return pixels_[y * width_ + x];
    }

    int width() const {
        return width_;
    }

    int height() const {
        return height_;
    }

private:
    int width_ = 0;
    int height_ = 0;

    std::vector<ColorPixel> pixels_;
};

}
