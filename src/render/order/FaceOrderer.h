#pragma once

#include <array>
#include <vector>

#include "../model/RenderMesh.h"
#include "FaceOrderMode.h"

namespace rf::render {

constexpr size_t PriorityBucketCount = 12;

using PriorityBuckets =
    std::array<
        std::vector<RenderFace>,
        PriorityBucketCount
    >;

class FaceOrderer {
public:
    void order(
        RenderMesh& mesh,
        FaceOrderMode mode
    ) const;

    PriorityBuckets buildPriorityBuckets(
        const RenderMesh& mesh
    ) const;

private:
    void orderRuneScapePriority(
        RenderMesh& mesh
    ) const;

    void orderDepthPainter(
        RenderMesh& mesh
    ) const;

    float averageBucketDepth(
        const std::vector<RenderFace>& a,
        const std::vector<RenderFace>& b
    ) const;
};

}
