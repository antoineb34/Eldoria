#pragma once

#include <cstdint>

#include "ModelAsset.h"
#include "ModelFile.h"
#include "ModelSourceMap.h"

namespace eld::model {

struct Model {
    std::uint16_t id = 0;

    ModelFile file;
    ModelAsset asset;
    ModelSourceMap sourceMap;
};

}
