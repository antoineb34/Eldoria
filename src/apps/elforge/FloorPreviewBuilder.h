#pragma once

#include "definition/floor/FloorDefinition.h"
#include "image/Image.h"

namespace eld::elforge {

class FloorPreviewBuilder {
public:
    eld::image::Image build(
        const eld::definition::FloorDefinition& floor
    ) const;
};

}
