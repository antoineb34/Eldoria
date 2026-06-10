#pragma once

#include "CacheTreeNode.h"
#include "cache/Cache.h"

namespace eld::elforge {

class CacheTreeBuilder {
public:
    CacheTreeNode build(const eld::cache::Cache& cache);
};

}
