#pragma once

#include "RenderQueue.h"

namespace rf::render_next {

class DepthSorter {
public:
    void sort(
        RenderQueue& queue
    ) const;
};

}
