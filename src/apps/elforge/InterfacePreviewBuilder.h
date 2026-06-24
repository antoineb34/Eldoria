#pragma once

#include "image/Image.h"
#include "interface/InterfaceDefinition.h"
#include "interface/InterfaceRepository.h"

namespace eld::elforge {

class InterfacePreviewBuilder {
public:
    eld::image::Image build(
        const eld::interface::InterfaceDefinition& root,
        const eld::interface::InterfaceRepository& repository
    ) const;
};

}
