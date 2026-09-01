#pragma once

#include <string>

#include "explorer/tree/CacheTreeNode.h"

namespace eld::elforge {

struct CacheSelection {
    CacheTreeNodeType type = CacheTreeNodeType::Root;

    std::string label = "Nothing selected";
    std::string key;

    std::string name;
    int frameId = -1;
    int definitionId = -1;
    int regionId = -1;
    int terrainFileId = -1;
    int objectFileId = -1;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    bool hasSelection() const {
        return indexId >= 0 || archiveId >= 0 || fileId >= 0;    }
};

}
