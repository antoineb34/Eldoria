#include "VisibilityStage.h"

#include <vector>

namespace rf::render_next {

namespace {

float screenArea(
    const rf::render::ScreenPoint& a,
    const rf::render::ScreenPoint& b,
    const rf::render::ScreenPoint& c
) {
    return
        (b.x - a.x) * (c.y - a.y) -
        (b.y - a.y) * (c.x - a.x);
}

bool isFrontFacing(
    const RenderPacket& packet,
    const ProjectedMesh& mesh
) {
    const rf::render::ScreenPoint& a =
        mesh.vertices[packet.a].screen;

    const rf::render::ScreenPoint& b =
        mesh.vertices[packet.b].screen;

    const rf::render::ScreenPoint& c =
        mesh.vertices[packet.c].screen;

    return screenArea(a, b, c) < 0.0f;
}

}

void VisibilityStage::apply(
    RenderQueue& queue,
    const ProjectedMesh& mesh
) const {
    std::vector<RenderPacket> visible;
    visible.reserve(queue.packets.size());

    for (const RenderPacket& packet : queue.packets) {
        if (!isFrontFacing(packet, mesh)) {
            continue;
        }

        visible.push_back(packet);
    }

    queue.packets = std::move(visible);
}

}
