#pragma once

#include "math/Vec3.h"
#include "math/Mat4.h"

namespace eld::render {

struct Transform {
    eld::math::Vec3 position { 0.0f, 0.0f, 0.0f };
    eld::math::Vec3 rotation { 0.0f, 0.0f, 0.0f };
    eld::math::Vec3 scale { 1.0f, 1.0f, 1.0f };
};

eld::math::Mat4 buildModelMatrix(
    const Transform& transform
);

}
