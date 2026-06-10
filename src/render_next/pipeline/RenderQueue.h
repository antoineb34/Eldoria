#pragma once

#include <vector>

#include "RenderPacket.h"

namespace eld::render_next {

struct RenderQueue {
    std::vector<RenderPacket> packets;

    void clear() {
        packets.clear();
    }

    bool empty() const {
        return packets.empty();
    }

    size_t size() const {
        return packets.size();
    }
};

}
