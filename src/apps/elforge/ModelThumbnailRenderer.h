#pragma once

#include <cstdint>

#include "graphics/GraphicsResources.h"
#include "graphics/model/ModelHandle.h"
#include "image/Image.h"

namespace eld::elforge {

class ModelThumbnailRenderer {
public:
    eld::image::Image render(
        eld::graphics::ModelHandle model,
        const eld::graphics::GraphicsResources& resources,
        std::uint16_t width,
        std::uint16_t height,
        std::uint16_t zoom,
        std::uint16_t rotationX,
        std::uint16_t rotationY
    ) const;
};

}
