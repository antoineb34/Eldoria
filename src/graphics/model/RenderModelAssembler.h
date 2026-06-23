#pragma once

#include "RenderModel.h"
#include "model/ModelMesh.h"
#include "texture/TextureResolver.h"

namespace eld::graphics {

class RenderModelAssembler {
public:
    explicit RenderModelAssembler(
        TextureResolver& textureResolver
    );

    RenderModel assemble(
        const eld::model::ModelMesh& source
    );

private:
    TextureResolver& textureResolver_;
};

}
