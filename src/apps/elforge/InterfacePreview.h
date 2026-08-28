#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "interface/InterfaceFile.h"
#include "math/Vec3.h"

namespace eld::elforge {

struct InterfacePreviewModel {
    std::uint16_t modelId = 0;

    eld::math::Vec3 rotation{
        0.0f,
        0.0f,
        0.0f
    };

    float depth = 0.0f;
};

struct InterfacePreviewNode {
    eld::interface::InterfaceFileWidget widget;

    int x = 0;
    int y = 0;

    std::optional<InterfacePreviewModel> model;

    std::vector<InterfacePreviewNode> children;
};

struct InterfacePreview {
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

    InterfacePreviewNode root;
};

}
