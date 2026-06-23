#pragma once

#include <cstddef>
#include <vector>

#include "RenderItem.h"

namespace eld::render {

struct RenderQueue {
    std::vector<RenderItem> items;

    void clear() {
        items.clear();
    }

    bool empty() const {
        return items.empty();
    }

    std::size_t size() const {
        return items.size();
    }
};

}
