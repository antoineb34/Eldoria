#pragma once

#include <string>
#include <vector>

namespace eld::elforge {

enum class CacheTreeNodeType {
    Root,
    Index,
    File,
    Model,
    Texture
};

struct CacheTreeNode {
    CacheTreeNodeType type = CacheTreeNodeType::File;

    std::string label;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    std::vector<CacheTreeNode> children;
};

}
