#include "CacheTreeBuilder.h"

#include <string>
#include <vector>

namespace rf::explorer {

namespace {

CacheTreeNode makeRoot() {
    CacheTreeNode node;
    node.type = CacheTreeNodeType::Root;
    node.label = "Cache";
    return node;
}

CacheTreeNode makeIndexNode(
    rf::cache::CacheIndex index,
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
    rf::cache::CacheIndex index,
    const rf::cache::CacheFile& file
) {
    CacheTreeNode node;

    node.type =
        index == rf::cache::CacheIndex::Model
            ? CacheTreeNodeType::Model
            : CacheTreeNodeType::File;

    node.label =
        "File " + std::to_string(file.id);

    if (index == rf::cache::CacheIndex::Model) {
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
    const rf::cache::Cache& cache,
    rf::cache::CacheIndex index,
    const std::string& label
) {
    CacheTreeNode indexNode =
        makeIndexNode(
            index,
            label
        );

    std::vector<rf::cache::CacheFile> files =
        cache.listFiles(index);

    for (const rf::cache::CacheFile& file : files) {
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
    const rf::cache::Cache& cache
) {
    CacheTreeNode root =
        makeRoot();

    addIndex(
        root,
        cache,
        rf::cache::CacheIndex::Config,
        "Index 0 - Config"
    );

    addIndex(
        root,
        cache,
        rf::cache::CacheIndex::Model,
        "Index 1 - Models"
    );

    addIndex(
        root,
        cache,
        rf::cache::CacheIndex::Animation,
        "Index 2 - Animations"
    );

    addIndex(
        root,
        cache,
        rf::cache::CacheIndex::Midi,
        "Index 3 - Midi"
    );

    addIndex(
        root,
        cache,
        rf::cache::CacheIndex::Map,
        "Index 4 - Maps"
    );

    return root;
}

}
