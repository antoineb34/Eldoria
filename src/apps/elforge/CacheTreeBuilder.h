#pragma once

#include "CacheTreeNode.h"
#include "cache_legacy/Cache.h"

namespace eld::elforge {

class CacheTreeBuilder {
public:
    CacheTreeNode build(const eld::cache_legacy::Cache& cache);
};

}
