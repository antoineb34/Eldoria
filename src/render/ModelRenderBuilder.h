#pragma once

#include <cstdint>
#include <unordered_map>

#include "RenderModel.h"

#include "model/ModelAsset.h"
#include "texture/TextureAsset.h"

namespace eld::render {

class ModelRenderBuilder {
public:
    RenderModel build(
        const eld::model::ModelMesh& model,
        const std::unordered_map<
            std::uint16_t,
            const eld::texture::TextureAsset*
        >& textures
    ) const;
};

}
