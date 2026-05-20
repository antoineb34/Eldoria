#include "DepthBuffer.h"

#include <limits>

namespace rf::render {

DepthBuffer::DepthBuffer(int width, int height) {
    resize(width, height);
}

void DepthBuffer::resize(int width, int height) {
    width_ = width;
    height_ = height;

    values_.resize(
        width_ * height_
    );

    clear();
}

void DepthBuffer::clear() {
    std::fill(
        values_.begin(),
        values_.end(),
        -std::numeric_limits<float>::infinity()
    );
}

bool DepthBuffer::testAndSet(int x, int y, float depth) {
    if (
        x < 0 ||
        y < 0 ||
        x >= width_ ||
        y >= height_
    ) {
        return false;
    }

    int index =
        y * width_ + x;

    if (depth <= values_[index]) {
        return false;
    }

    values_[index] = depth;

    return true;
}

}
