#pragma once

#include <vector>

#include "../../render/software/camera/Projection.h"
#include "../../render/software/math/Vec3.h"

namespace rf::render_next {

struct ProjectedVertex {
    rf::render::Vec3 world;
    rf::render::Vec3 view;
    rf::render::ScreenPoint screen;

    bool valid = true;
};

struct ProjectedMesh {
    std::vector<ProjectedVertex> vertices;
};

}
