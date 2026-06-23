#pragma once

#include <string>

#include "CacheTreeNode.h"

namespace eld::elforge {

struct CacheSelection {
    CacheTreeNodeType type = CacheTreeNodeType::Root;

    std::string label = "Nothing selected";
    std::string key;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    bool hasSelection() const {
        return indexId >= 0 || archiveId >= 0 || fileId >= 0;    }
};

}
