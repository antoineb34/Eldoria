#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace eld::image {

struct IndexedImagePixelSource {
    std::size_t filePixelIndex = 0;
    std::size_t paletteIndex = 0;
};

struct IndexedImageSourceMap {
    std::vector<
        std::optional<IndexedImagePixelSource>
    > pixels;
};

}
