#pragma once

#include "../geometry/RenderSubmesh.h"
#include "../material/Material.h"
#include "../scene/RenderObject.h"

namespace eld::render {

struct RenderItem {
    const RenderObject* object = nullptr;
    const RenderSubmesh* submesh = nullptr;
    const Material* material = nullptr;

    float depth = 0.0f;
};

}
