#pragma once

#include "RenderModel.h"
#include "model/ModelMesh.h"

namespace eld::graphics {

class RenderModelBuilder {
public:
    RenderModel build(
        const eld::model::ModelMesh& source
    ) const;
};

}
