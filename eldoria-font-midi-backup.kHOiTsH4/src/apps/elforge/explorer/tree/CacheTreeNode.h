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
        Animation,
        Texture,
        Sprite,
        SpriteFrame,
        Image,
        Font,
        DefinitionGroup,
        Floor,
        IdentityKit,
        Location,
        Npc,
        Item,
        Sequence,
        SpotAnimation,
        Varp,
        Varbit,
        Parameter,
        Message,
        MessageAnimation,
        Widget,
        Midi,
        MapRegion,
    };

struct CacheTreeNode {
    CacheTreeNodeType type = CacheTreeNodeType::File;

    std::string label;
    std::string key;

    std::string name;
    int frameId = -1;
    int definitionId = -1;
    int regionId = -1;
    int terrainFileId = -1;
    int locationFileId = -1;

    int indexId = -1;
    int archiveId = -1;
    int fileId = -1;

    std::vector<CacheTreeNode> children;
};

}
