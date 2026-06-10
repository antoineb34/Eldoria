#pragma once

#include "RenderQueue.h"

namespace eld::render_next {

class DepthSorter {
public:
    void sort(
        RenderQueue& queue
    ) const;
};

}
