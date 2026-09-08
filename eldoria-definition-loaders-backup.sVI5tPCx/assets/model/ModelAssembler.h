#pragma once

#include <cstdint>
#include <map>

#include "model/ModelData.h"
#include "model/ModelResource.h"
#include "texture/TextureResource.h"

namespace eld::model {

class ModelAssembler {
public:
    ModelResource assemble(
        ModelData data,
        std::map<
            std::uint16_t,
            eld::texture::TextureResource
        > textures
    ) const;
};

}
