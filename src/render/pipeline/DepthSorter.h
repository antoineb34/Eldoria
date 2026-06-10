#pragma once

#include "RenderQueue.h"

namespace eld::render {

class DepthSorter {
public:
    void sort(
        RenderQueue& queue
    ) const;
};

}
