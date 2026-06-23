#pragma once

#include <vector>

#include "../../camera/Projection.h"
#include "../../math/Vec2.h"
#include "../../math/Vec3.h"
#include "../../math/Vec4.h"
#include "../../scene/RenderCamera.h"
#include "../../scene/RenderObject.h"

namespace eld::render {

struct SoftwareProjectedVertex {
    Vec3 world;
    Vec3 view;

    ScreenPoint screen;

    Vec2 uv;
    Vec4 color;

    bool valid = true;
};

struct SoftwareProjectedMesh {
    std::vector<SoftwareProjectedVertex> vertices;
};

class SoftwareMeshProjector {
public:
    SoftwareProjectedMesh project(
        const RenderObject& object,
        const RenderCamera& camera
    ) const;
};

}
