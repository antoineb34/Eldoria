#pragma once

#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Vec4.h"

namespace eld::render {

struct RenderVertex {
    Vec3 position;
    Vec3 normal;

    Vec2 uv;

    Vec4 color{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };
};

}
