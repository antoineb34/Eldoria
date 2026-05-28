#pragma once

#include <vector>

namespace rf::render {

class DepthBuffer {
public:
    DepthBuffer(
        int width,
        int height
    );

    void resize(
        int width,
        int height
    );

    void clear();

    bool testAndSet(
        int x,
        int y,
        float depth
    );

private:
    int width_ = 0;
    int height_ = 0;

    std::vector<float> values_;
};

}
