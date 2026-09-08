#include "model/ModelAssembler.h"

#include <utility>

namespace eld::model {

ModelResource ModelAssembler::assemble(
    ModelData data,
    std::map<
        std::uint16_t,
        eld::texture::TextureResource
    > textures
) const {
    return ModelResource{
        .data = std::move(data),
        .textures = std::move(textures)
    };
}

}
