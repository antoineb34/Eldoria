#pragma once

#include "ModelAsset.h"
#include "ModelFile.h"

namespace eld::model {

class ModelBuilder {
public:
    ModelAsset build(
        const ModelFile& file
    ) const;
};

}
