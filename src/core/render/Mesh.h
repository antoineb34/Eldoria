#pragma once

#include <vector>
#include "render/Vertex.h"
#include "assets/model/Model.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static Mesh fromModel(const Model& model);
};
