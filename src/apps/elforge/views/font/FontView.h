#pragma once

#include "font/FontData.h"
#include "image/ImageData.h"

namespace eld::elforge {

class FontView {
public:
  eld::image::ImageData build(const eld::font::FontData &font) const;
};

} // namespace eld::elforge
