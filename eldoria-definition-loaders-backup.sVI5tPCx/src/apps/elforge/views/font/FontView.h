#pragma once

#include "Font.h"
#include "image/ImageData.h"

namespace eld::elforge {

class FontView {
public:
    eld::image::ImageData build(
        const eld::font::Font& font
    ) const;
};

}
