#pragma once

#include <vector>
#include "render/Vertex.h"
#include "assets/model/ModelDef.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static Mesh FromModelDef(const ModelDef& def);
};
