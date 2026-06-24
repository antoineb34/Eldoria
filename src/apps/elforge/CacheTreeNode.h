#pragma once

#include <string>
#include <vector>

namespace eld::elforge {

    enum class CacheTreeNodeType {
        Root,
        Index,
        Archive,
        File,
        ArchiveFile,
        Model,
        Texture,
        Sprite,
        SpriteFrame,
        Image,
        Font,
        DefinitionGroup,
        FloorDefinition,
        IdentityKitDefinition,
        LocationDefinition,
        NpcDefinition,
        ItemDefinition,
    };

struct CacheTreeNode {
    CacheTreeNodeType type = CacheTreeNodeType::File;

    std::string label;
    std::string key;

    std::string name;
    int frameId = -1;
    int definitionId = -1;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    std::vector<CacheTreeNode> children;
};

}
