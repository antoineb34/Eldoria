#pragma once

#include "../../render/software/math/Vec3.h"
#include "../../render/software/math/Mat4.h"

namespace eld::render_next {

struct Transform {
    eld::render::Vec3 position { 0.0f, 0.0f, 0.0f };
    eld::render::Vec3 rotation { 0.0f, 0.0f, 0.0f };
    eld::render::Vec3 scale { 1.0f, 1.0f, 1.0f };
};

eld::render::Mat4 buildModelMatrix(
    const Transform& transform
);

}
