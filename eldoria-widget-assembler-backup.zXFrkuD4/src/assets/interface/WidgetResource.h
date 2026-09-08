#pragma once

#include <cstdint>
#include <vector>

#include "interface/WidgetData.h"

namespace eld::interface {

struct WidgetNode {
    WidgetData data;

    std::int16_t x = 0;
    std::int16_t y = 0;

    std::vector<WidgetNode> children;
};


struct WidgetResource {
    std::uint16_t rootId = 0;
    WidgetNode root;
};

}
