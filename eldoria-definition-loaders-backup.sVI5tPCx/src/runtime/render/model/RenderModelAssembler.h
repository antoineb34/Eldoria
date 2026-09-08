#pragma once

#include "RenderModel.h"
#include "model/ModelData.h"
#include "texture/TextureResolver.h"

namespace eld::render {

class RenderModelAssembler {
public:
    explicit RenderModelAssembler(
        TextureResolver& textureResolver
    );

    RenderModel assemble(
        const eld::model::ModelData& source
    );

private:
    TextureResolver& textureResolver_;
};

}
