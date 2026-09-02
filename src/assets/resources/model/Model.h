#pragma once

#include <cstdint>

#include "ModelFile.h"
#include "ModelMesh.h"
#include "ModelSourceMap.h"

namespace eld::model {

struct Model {
    std::uint16_t id = 0;

    ModelFile file;
    ModelMesh mesh;
    ModelSourceMap sourceMap;
};

}
