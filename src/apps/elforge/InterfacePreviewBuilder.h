#pragma once

#include "image/Image.h"
#include "interface/InterfaceDefinition.h"
#include "interface/InterfaceRepository.h"
#include "sprite/SpriteRepository.h"
#include "font/FontRepository.h"
#include "graphics/GraphicsResources.h"

namespace eld::elforge {

class InterfacePreviewBuilder {
public:
    eld::image::Image build(
        const eld::interface::InterfaceDefinition& root,
        const eld::interface::InterfaceRepository& repository,
        const eld::sprite::SpriteRepository& spriteRepository,
        const eld::font::FontRepository& fontRepository,
        eld::graphics::GraphicsResources& graphicsResources
    ) const;
};

}
