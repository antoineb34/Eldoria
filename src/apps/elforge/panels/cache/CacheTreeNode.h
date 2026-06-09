#pragma once

#include <string>
#include <vector>
#include "CacheTreeNodeType.h"

namespace eldoria::apps::elforge {

struct CacheTreeNode {
    CacheTreeNodeType type = CacheTreeNodeType::File;

    std::string label;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    std::vector<CacheTreeNode> children;
};

}
