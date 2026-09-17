#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace eld::render {

class DepthBuffer {
public:
    void resize(
        std::uint32_t width,
        std::uint32_t height
    ) {
        width_ = width;
        height_ = height;

        depths_.resize(
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height)
        );
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
        if (
            x < 0 ||
            y < 0 ||
            static_cast<std::uint32_t>(x) >= width_ ||
            static_cast<std::uint32_t>(y) >= height_
        ) {
            return false;
        }

        float& current =
            depths_.at(
                static_cast<std::size_t>(y) * width_ +
                static_cast<std::size_t>(x)
            );

        if (depth >= current) {
            return false;
        }

        current = depth;
        return true;
    }

    float at(
        std::uint32_t x,
        std::uint32_t y
    ) const {
        return depths_.at(
            static_cast<std::size_t>(y) * width_ + x
        );
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

    std::vector<float> depths_;
};

}
