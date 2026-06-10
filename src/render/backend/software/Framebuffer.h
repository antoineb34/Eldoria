#pragma once

#include "ColorBuffer.h"
#include "DepthBuffer.h"

namespace eld::render {

class Framebuffer {
public:
    void resize(
        int width,
        int height
    ) {
        color_.resize(width, height);
        depth_.resize(width, height);
    }

    void clear(
        ColorPixel color = { 169, 199, 151, 255 }
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
