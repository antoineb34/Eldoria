#include "texture/TextureAssembler.h"

#include <utility>

namespace eld::texture {

TextureResource TextureAssembler::assemble(
    std::uint16_t id,
    eld::image::ImageData data
) const {
    return TextureResource{
        .id = id,
        .image = std::move(data)
    };
}

}
