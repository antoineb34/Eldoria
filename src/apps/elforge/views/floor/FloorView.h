#pragma once

#include "floor/FloorData.h"
#include "image/ImageData.h"

namespace eld::elforge {

class FloorView {
public:
  eld::image::ImageData build(const eld::floor::FloorData &floor) const;
};

} // namespace eld::elforge
