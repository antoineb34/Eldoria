#pragma once

#include "../math/Vec3.h"
#include "../math/Mat4.h"
#include "Camera.h"
#include "model/ModelAsset.h"

namespace rf::render {

struct ScreenPoint {

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Vec3 toVec3(
    const rf::model::Vertex& vertex
);

Mat4 buildViewMatrix(
    const Camera& camera
);

Mat4 buildProjectionMatrix(
    const Camera& camera
);

ScreenPoint projectPoint(
    const Vec3& point,
    const Mat4& view,
    const Mat4& projection,
    const Camera& camera

);

ScreenPoint projectVertex(
    const rf::model::Vertex& vertex,
    const Mat4& view,
    const Mat4& projection,
    const Camera& camera
);

}
