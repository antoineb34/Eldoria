#include "FaceOrderer.h"

#include <algorithm>

namespace rf::render {

void FaceOrderer::order(
    RenderMesh& mesh,
    FaceOrderMode mode
) const {
    switch (mode) {
        case FaceOrderMode::RuneScapePriority:
            orderRuneScapePriority(mesh);
            break;

        case FaceOrderMode::DepthPainter:
            orderDepthPainter(mesh);
            break;
    }
}

PriorityBuckets FaceOrderer::buildPriorityBuckets(
    const RenderMesh& mesh
) const {
    PriorityBuckets buckets;

    for (const RenderFace& face : mesh.faces) {
        size_t priority =
            static_cast<size_t>(face.source->priority);

        if (priority >= PriorityBucketCount) {
            priority = 0;
        }

        buckets[priority].push_back(face);
    }

    return buckets;
}

void FaceOrderer::orderDepthPainter(
    RenderMesh& mesh
) const {
    std::sort(
        mesh.faces.begin(),
        mesh.faces.end(),
        [](const RenderFace& a, const RenderFace& b) {
            return a.depthAvg < b.depthAvg;
        }
    );
}

void FaceOrderer::orderRuneScapePriority(
    RenderMesh& mesh
) const {
    // RuneScape priority rendering still uses priority buckets later.
    // This pre-sorts faces by depth so each bucket inherits depth order.
    orderDepthPainter(mesh);
}

}
