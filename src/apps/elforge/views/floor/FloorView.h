#pragma once

#include "definition/floor/FloorDefinition.h"
#include "image/Image.h"

namespace eld::elforge {

class FloorView {
public:
    eld::image::Image build(
        const eld::definition::FloorDefinition& floor
    ) const;
};

}
