#pragma once

#include <cstdint>
#include <map>

#include "Model.h"
#include "texture/Texture.h"

namespace eld::model {

struct ModelResource {
    Model model;

    std::map<
        std::uint16_t,
        eld::texture::Texture
    > textures;
};

}
