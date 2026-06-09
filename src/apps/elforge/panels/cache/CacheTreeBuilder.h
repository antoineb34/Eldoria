#pragma once

#include "CacheTreeNode.h"
#include "cache/Cache.h"

namespace eldoria::apps::elforge {

class CacheTreeBuilder {
public:
    CacheTreeNode build(const rf::cache::Cache& cache);
};

}
