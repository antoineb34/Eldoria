#include "CacheTreeBuilder.h"

#include <string>
#include <vector>

namespace eld::elforge {

namespace {

CacheTreeNode makeRoot() {
    CacheTreeNode node;
    node.type = CacheTreeNodeType::Root;
    node.label = "Cache";
    return node;
}

CacheTreeNode makeIndexNode(
    eld::cache_legacy::CacheIndex index,
    const std::string& label
) {
    CacheTreeNode node;

    node.type = CacheTreeNodeType::Index;
    node.label = label;

    node.indexId =
        static_cast<int>(index);

    node.archiveId = -1;
    node.fileId = -1;

    return node;
}

CacheTreeNode makeFileNode(
    eld::cache_legacy::CacheIndex index,
    const eld::cache_legacy::CacheFile& file
) {
    CacheTreeNode node;

    node.type =
        index == eld::cache_legacy::CacheIndex::Model
            ? CacheTreeNodeType::Model
            : CacheTreeNodeType::File;

    node.label =
        "File " + std::to_string(file.id);

    if (index == eld::cache_legacy::CacheIndex::Model) {
        node.label =
            "Model " + std::to_string(file.id);
    }

    node.indexId =
        static_cast<int>(index);

    node.archiveId = -1;
    node.fileId = file.id;

    return node;
}

void addIndex(
    CacheTreeNode& root,
    const eld::cache_legacy::Cache& cache,
    eld::cache_legacy::CacheIndex index,
    const std::string& label
) {
    CacheTreeNode indexNode =
        makeIndexNode(
            index,
            label
        );

    std::vector<eld::cache_legacy::CacheFile> files =
        cache.listFiles(index);

    for (const eld::cache_legacy::CacheFile& file : files) {
        indexNode.children.push_back(
            makeFileNode(
                index,
                file
            )
        );
    }

    root.children.push_back(
        indexNode
    );
}

}

CacheTreeNode CacheTreeBuilder::build(
    const eld::cache_legacy::Cache& cache
) {
    CacheTreeNode root =
        makeRoot();

    addIndex(
        root,
        cache,
        eld::cache_legacy::CacheIndex::Config,
        "Index 0 - Config"
    );

    addIndex(
        root,
        cache,
        eld::cache_legacy::CacheIndex::Model,
        "Index 1 - Models"
    );

    addIndex(
        root,
        cache,
        eld::cache_legacy::CacheIndex::Animation,
        "Index 2 - Animations"
    );

    addIndex(
        root,
        cache,
        eld::cache_legacy::CacheIndex::Midi,
        "Index 3 - Midi"
    );

    addIndex(
        root,
        cache,
        eld::cache_legacy::CacheIndex::Map,
        "Index 4 - Maps"
    );

    return root;
}

}
