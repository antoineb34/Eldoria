#pragma once

#include <cstdint>

#include "math/Vec3.h"

namespace eld::render {

struct Camera {
    eld::math::Vec3 position{
        0.0f,
        0.0f,
        0.0f
    };

    eld::math::Vec3 rotation{
        0.0f,
        0.0f,
        0.0f
    };

    float verticalFov = 1.04719755f;

    float nearPlane = 1.0f;
    float farPlane = 10000.0f;

    std::uint32_t viewportWidth = 800;
    std::uint32_t viewportHeight = 600;
};

}
