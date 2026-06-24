#pragma once

#include "font/Font.h"
#include "image/Image.h"

namespace eld::elforge {

class FontPreviewBuilder {
public:
    eld::image::Image build(
        const eld::font::Font& font
    ) const;
};

}
