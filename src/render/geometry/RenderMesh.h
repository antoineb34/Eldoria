#pragma once

#include <cstdint>
#include <vector>

#include "RenderSubmesh.h"
#include "RenderVertex.h"

namespace eld::render {

struct RenderMesh {
    std::vector<RenderVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<RenderSubmesh> submeshes;
};

}
