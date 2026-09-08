#pragma once

#include "Floor.h"
#include "image/ImageData.h"

namespace eld::elforge {

class FloorView {
public:
    eld::image::ImageData build(
        const eld::floor::Floor& floor
    ) const;
};

}
