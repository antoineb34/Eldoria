#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace eld::texture {

struct TexturePixelSource {
    std::size_t filePixelIndex = 0;
    std::size_t paletteIndex = 0;
};

struct TextureSourceMap {
    std::vector<
        std::optional<TexturePixelSource>
    > pixels;
};

}
