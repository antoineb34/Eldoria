#include "DepthSorter.h"

#include <algorithm>

namespace eld::render_next {

void DepthSorter::sort(
    RenderQueue& queue
) const {
    std::sort(
        queue.packets.begin(),
        queue.packets.end(),
        [](const RenderPacket& a, const RenderPacket& b) {
            return a.depthAvg > b.depthAvg;
        }
    );
}

}
