#pragma once

#include <vector>

#include "../../render/camera/Projection.h"
#include "../../render/math/Vec3.h"

namespace eld::render {

struct ProjectedVertex {
    eld::render::Vec3 local;
    eld::render::Vec3 world;
    eld::render::Vec3 view;
    eld::render::ScreenPoint screen;

    bool valid = true;
};

struct ProjectedMesh {
    std::vector<ProjectedVertex> vertices;
};

}
