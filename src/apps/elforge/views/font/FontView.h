#pragma once

#include "Font.h"
#include "Image.h"

namespace eld::elforge {

class FontView {
public:
    eld::image::Image build(
        const eld::font::Font& font
    ) const;
};

}
