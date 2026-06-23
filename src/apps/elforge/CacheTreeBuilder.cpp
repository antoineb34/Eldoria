#include "CacheTreeBuilder.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "archive/ArchiveHashes.h"
#include "archive/ArchiveParser.h"
#include "cache/Store.h"

namespace eld::elforge {

namespace {

std::string getIndexLabel(
    eld::cache::IndexId index
) {
    switch (index) {
        case eld::cache::IndexId::Config:
            return "Index 0 - Config";

        case eld::cache::IndexId::Models:
            return "Index 1 - Models";

        case eld::cache::IndexId::Animations:
            return "Index 2 - Animations";

        case eld::cache::IndexId::Midi:
            return "Index 3 - Midi";

        case eld::cache::IndexId::Maps:
            return "Index 4 - Maps";
    }

    return "Unknown Index";
}

CacheTreeNode makeRoot() {
    CacheTreeNode node;

    node.type = CacheTreeNodeType::Root;
    node.key = "cache";
    node.label = "Cache";

    return node;
}

CacheTreeNode makeIndexNode(
    eld::cache::IndexId index
) {
    CacheTreeNode node;

    node.type = CacheTreeNodeType::Index;
    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        );

    node.label =
        getIndexLabel(index);

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

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/file/" +
        std::to_string(entry.fileId);

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

CacheTreeNode makeArchiveFileNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::ArchiveFile;

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/archive/" +
        std::to_string(archiveId) +
        "/file/" +
        std::to_string(file.id);

    const std::optional<std::string_view> name =
        eld::archive::findName(
            file.nameHash
        );

    if (name.has_value()) {
        node.label =
            std::string(*name);
    }
    else {
        node.label =
            "File " +
            std::to_string(file.id) +
            " - Hash " +
            std::to_string(file.nameHash);
    }

    node.indexId =
        static_cast<int>(index);

    node.archiveId =
        static_cast<int>(archiveId);

    node.fileId =
        static_cast<int>(file.id);

    return node;
}

std::optional<CacheTreeNode> makeArchiveNode(
    eld::cache::IndexId index,
    const eld::cache::Store& store,
    const eld::cache::FileEntry& entry
) {
    const eld::cache::File cacheFile =
        store.get(
            entry.fileId
        );

    eld::archive::ArchiveParser parser;

    std::optional<eld::archive::Archive> archive =
        parser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        return std::nullopt;
    }

    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Archive;

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/archive/" +
        std::to_string(entry.fileId);

    node.label =
        "Archive " +
        std::to_string(entry.fileId) +
        " (" +
        std::to_string(archive->count()) +
        " files)";

    node.indexId =
        static_cast<int>(index);

    node.archiveId =
        static_cast<int>(entry.fileId);

    for (
        const eld::archive::ArchiveFile& file :
        archive->list()
    ) {
        node.children.push_back(
            makeArchiveFileNode(
                index,
                entry.fileId,
                file
            )
        );
    }

    return node;
}

void addIndex(
    CacheTreeNode& root,
    const eld::cache::Cache& cache,
    const eld::cache::Index& index
) {
    CacheTreeNode indexNode =
        makeIndexNode(
            index.id
        );

    const eld::cache::Store store =
        cache.open(
            index.id
        );

    const std::vector<eld::cache::FileEntry> entries =
        store.list();

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        if (
            index.id ==
            eld::cache::IndexId::Config
        ) {
            const std::optional<CacheTreeNode> archive =
                makeArchiveNode(
                    index.id,
                    store,
                    entry
                );

            if (archive.has_value()) {
                indexNode.children.push_back(
                    *archive
                );

                continue;
            }
        }

        indexNode.children.push_back(
            makeFileNode(
                index.id,
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

    for (
        const eld::cache::Index& index :
        cache.list()
    ) {
        addIndex(
            root,
            cache,
            index
        );
    }

    return root;
}

}
