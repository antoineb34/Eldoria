#pragma once

#include "ModelAsset.h"
#include "ModelFile.h"

namespace rf::model {

class ModelBuilder {
public:
    ModelAsset build(
        const ModelFile& file
    ) const;
};

}
