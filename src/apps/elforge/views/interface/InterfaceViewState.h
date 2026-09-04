#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Widget.h"
#include "math/Vec3.h"

namespace eld::elforge {

struct InterfaceViewModel {
    std::uint16_t modelId = 0;

    eld::math::Vec3 rotation{
        0.0f,
        0.0f,
        0.0f
    };

    float depth = 0.0f;
};

struct InterfaceViewNode {
    eld::interface::Widget widget;

    int x = 0;
    int y = 0;

    std::optional<InterfaceViewModel> model;

    std::vector<InterfaceViewNode> children;
};

struct InterfaceViewState {
    std::uint16_t rootId = 0;

    int width = 0;
    int height = 0;

    // RS317 interface models project with:
    //
    //     screen = center + coordinate * 512 / depth
    //
    // Keep that source semantic here. The renderer converts it to
    // a normal perspective camera.
    float modelProjectionFocalLength = 512.0f;

    InterfaceViewNode root;
};

}
