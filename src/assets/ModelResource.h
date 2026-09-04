#pragma once

#include <cstdint>
#include <map>

#include "Model.h"
#include "Texture.h"

namespace eld::model {

struct ModelResource {
    Model model;

    std::map<
        std::uint16_t,
        eld::texture::Texture
    > textures;
};

}
