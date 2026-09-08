#pragma once

#include <vector>

#include "camera/Camera.h"
#include "camera/Projection.h"
#include "render/model/RenderModel.h"
#include "math/Vec2.h"
#include "math/Vec4.h"
#include "scene/Transform.h"

namespace eld::render {

struct SoftwareProjectedVertex {
    ScreenPoint screen;

    eld::math::Vec2 uv;
    eld::math::Vec4 color;

    bool valid = false;
};

struct SoftwareProjectedMesh {
    std::vector<SoftwareProjectedVertex> vertices;
};

class SoftwareMeshProjector {
public:
    SoftwareProjectedMesh project(
        const eld::render::RenderMesh& mesh,
        const Transform& transform,
        const Camera& camera
    ) const;
};

}
