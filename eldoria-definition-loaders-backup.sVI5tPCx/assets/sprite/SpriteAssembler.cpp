#include "sprite/SpriteAssembler.h"

#include <utility>

namespace eld::sprite {

SpriteResource SpriteAssembler::assemble(
    std::string groupName,
    std::uint16_t frameId,
    eld::image::ImageData data
) const {
    return SpriteResource{
        .groupName = std::move(groupName),
        .frameId = frameId,
        .image = std::move(data)
    };
}

}
