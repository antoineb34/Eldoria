#include "CacheTreeBuilder.h"

#include <string>
#include <vector>

#include "cache/Store.h"

namespace eld::elforge {

namespace {

CacheTreeNode makeRoot() {
    CacheTreeNode node;

    node.type = CacheTreeNodeType::Root;
    node.label = "Cache";

    return node;
}

CacheTreeNode makeIndexNode(
    eld::cache::IndexId index,
    const std::string& label
) {
    CacheTreeNode node;

    node.type = CacheTreeNodeType::Index;
    node.label = label;

    node.indexId =
        static_cast<int>(index);

    return node;
}

CacheTreeNode makeFileNode(
    eld::cache::IndexId index,
    const eld::cache::FileEntry& entry
) {
    CacheTreeNode node;

    node.type =
        index == eld::cache::IndexId::Models
            ? CacheTreeNodeType::Model
            : CacheTreeNodeType::File;

    node.label =
        index == eld::cache::IndexId::Models
            ? "Model " +
                std::to_string(entry.fileId)
            : "File " +
                std::to_string(entry.fileId);

    node.indexId =
        static_cast<int>(index);

    node.fileId =
        static_cast<int>(entry.fileId);

    return node;
}

void addIndex(
    CacheTreeNode& root,
    const eld::cache::Cache& cache,
    eld::cache::IndexId index,
    const std::string& label
) {
    CacheTreeNode indexNode =
        makeIndexNode(
            index,
            label
        );

    const eld::cache::Store store =
        cache.open(
            index
        );

    const std::vector<eld::cache::FileEntry> entries =
        store.list();

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        indexNode.children.push_back(
            makeFileNode(
                index,
                entry
            )
        );
    }

    root.children.push_back(
        std::move(indexNode)
    );
}

}

CacheTreeNode CacheTreeBuilder::build(
    const eld::cache::Cache& cache
) const {
    CacheTreeNode root =
        makeRoot();

    addIndex(
        root,
        cache,
        eld::cache::IndexId::Config,
        "Index 0 - Config"
    );

    addIndex(
        root,
        cache,
        eld::cache::IndexId::Models,
        "Index 1 - Models"
    );

    addIndex(
        root,
        cache,
        eld::cache::IndexId::Animations,
        "Index 2 - Animations"
    );

    addIndex(
        root,
        cache,
        eld::cache::IndexId::Midi,
        "Index 3 - Midi"
    );

    addIndex(
        root,
        cache,
        eld::cache::IndexId::Maps,
        "Index 4 - Maps"
    );

    return root;
}

}
