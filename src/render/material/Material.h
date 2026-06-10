#pragma once

#include <cstdint>

namespace eld::render {

struct Material {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    bool textured = false;
};

}
