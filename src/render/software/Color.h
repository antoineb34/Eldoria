#pragma once

#include <cstdint>

namespace rf::render {

struct RgbColor {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
};

RgbColor rsColorToRgb(
    uint16_t color
);

}
