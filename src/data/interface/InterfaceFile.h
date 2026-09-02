#pragma once

#include <cstdint>
#include <vector>

#include "InterfaceWidget.h"

namespace eld::interface {

struct InterfaceFile {
    std::vector<std::uint8_t> payload;
    std::uint16_t declaredCount = 0;
    std::vector<InterfaceWidget> widgets;
};

}
