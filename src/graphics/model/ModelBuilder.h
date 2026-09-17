#pragma once

#include "model/ModelData.h"
#include "render/model/ModelResource.h"
#include "texture/TextureSystem.h"

namespace eld::graphics {

class ModelBuilder {
public:
    explicit ModelBuilder(
        TextureSystem& textures);

    eld::render::ModelResource build(
        const eld::model::ModelData& source);

private:
    TextureSystem& textures_;
};

}
