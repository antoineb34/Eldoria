#pragma once

#include <cstdint>
#include <map>

#include "model/ModelData.h"
#include "texture/TextureResource.h"

namespace eld::model {

struct ModelResource {
    ModelData data;

    std::map<
        std::uint16_t,
        eld::texture::TextureResource
    > textures;
};

}
