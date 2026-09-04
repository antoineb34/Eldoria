#pragma once

#include "Floor.h"
#include "Image.h"

namespace eld::elforge {

class FloorView {
public:
    eld::image::Image build(
        const eld::floor::Floor& floor
    ) const;
};

}
