#pragma once

#include <cstddef>
#include <string>

namespace eld::elforge::ui {

struct CarouselResult {
    bool previous = false;
    bool next = false;
};

CarouselResult carousel(
    const char* id,
    const char* label,
    const std::string& value,
    std::size_t index,
    std::size_t count,
    float width,
    const char* countTooltip = nullptr
);

}
