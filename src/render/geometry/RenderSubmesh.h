#pragma once

#include <cstddef>

namespace eld::render {

struct RenderSubmesh {
    std::size_t firstIndex = 0;
    std::size_t indexCount = 0;
    std::size_t materialIndex = 0;
};

}
