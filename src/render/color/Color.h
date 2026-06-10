#pragma once

#include <cstdint>

namespace eld::render {

struct RgbColor {

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

RgbColor rsColorToRgb(
    uint16_t color
);

}
