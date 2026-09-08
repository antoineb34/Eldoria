#pragma once

#include "Camera.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

namespace eld::render {

struct ScreenPoint {
    float x = 0.0f;
    float y = 0.0f;
    float depth = 0.0f;
};

eld::math::Mat4 buildViewMatrix(
    const Camera& camera
);

eld::math::Mat4 buildProjectionMatrix(
    const Camera& camera
);

ScreenPoint projectPoint(
    const eld::math::Vec3& worldPoint,
    const eld::math::Mat4& viewMatrix,
    const eld::math::Mat4& projectionMatrix,
    const Camera& camera
);

}
