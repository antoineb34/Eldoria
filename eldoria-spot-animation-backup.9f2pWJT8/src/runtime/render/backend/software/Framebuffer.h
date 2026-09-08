#pragma once

#include <cstdint>

#include "ColorBuffer.h"
#include "DepthBuffer.h"

namespace eld::render {

class Framebuffer {
public:
    void resize(
        std::uint32_t width,
        std::uint32_t height
    ) {
        color_.resize(width, height);
        depth_.resize(width, height);
    }

    void clear(
        ColorPixel color = {
            0,
            0,
            0,
            255
        }
    ) {
        color_.clear(color);
        depth_.clear();
    }

    ColorBuffer& color() {
        return color_;
    }

    const ColorBuffer& color() const {
        return color_;
    }

    DepthBuffer& depth() {
        return depth_;
    }

    const DepthBuffer& depth() const {
        return depth_;
    }

private:
    ColorBuffer color_;
    DepthBuffer depth_;
};

}
