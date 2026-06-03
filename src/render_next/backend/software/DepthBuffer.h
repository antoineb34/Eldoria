#pragma once

#include <algorithm>
#include <limits>
#include <vector>

namespace rf::render_next {

class DepthBuffer {
public:
    void resize(
        int width,
        int height
    ) {
        width_ = width;
        height_ = height;
        depths_.resize(width * height);
    }

    void clear() {
        std::fill(
            depths_.begin(),
            depths_.end(),
            std::numeric_limits<float>::infinity()
        );
    }

    bool testAndWrite(
        int x,
        int y,
        float depth
    ) {
        float& current =
            depths_[y * width_ + x];

        if (depth >= current) {
            return false;
        }

        current = depth;
        return true;
    }

    float at(
        int x,
        int y
    ) const {
        return depths_[y * width_ + x];
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

    std::vector<float> depths_;
};

}
