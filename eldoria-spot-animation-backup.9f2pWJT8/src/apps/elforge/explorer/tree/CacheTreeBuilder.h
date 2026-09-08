#pragma once

#include <cstdint>
#include <vector>

#include "explorer/tree/CacheTreeNode.h"
#include "cache/Cache.h"

namespace eld::elforge {

class CacheTreeBuilder {
public:
    CacheTreeNode build(
        const eld::cache::Cache& cache,
        const std::vector<std::uint16_t>& textureIds
    ) const;
};

}
