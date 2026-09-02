#pragma once

#include "RenderModel.h"
#include "model/Model.h"
#include "texture/TextureResolver.h"

namespace eld::render {

class RenderModelAssembler {
public:
    explicit RenderModelAssembler(
        TextureResolver& textureResolver
    );

    RenderModel assemble(
        const eld::model::Model& source
    );

private:
    TextureResolver& textureResolver_;
};

}
