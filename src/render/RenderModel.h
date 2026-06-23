#pragma once

#include <vector>

#include "geometry/RenderMesh.h"
#include "material/Material.h"

namespace eld::render {

struct RenderModel {
    RenderMesh mesh;
    std::vector<Material> materials;
};

}
