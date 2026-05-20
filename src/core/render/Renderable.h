#pragma once

#include "render/Mesh.h"
#include "math/Mat4.h"

struct Renderable {
    const Mesh* mesh;
    Mat4 transform;
};
