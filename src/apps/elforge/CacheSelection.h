#pragma once

#include <string>

#include "CacheTreeNode.h"

namespace eld::explorer {

struct CacheSelection {
    CacheTreeNodeType type = CacheTreeNodeType::Root;

    std::string label = "Nothing selected";

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    bool hasSelection() const {
        return indexId >= 0 || archiveId >= 0 || fileId >= 0;    }
};

}
