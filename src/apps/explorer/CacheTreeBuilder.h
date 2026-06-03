#pragma once

#include "CacheTreeNode.h"
#include "../../core/cache/Cache.h"

namespace rf::explorer {

class CacheTreeBuilder {
public:
    CacheTreeNode build(const rf::cache::Cache& cache);
};

}
