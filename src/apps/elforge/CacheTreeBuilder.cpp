#include "CacheTreeBuilder.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "archive/ArchiveHashes.h"
#include "archive/ArchiveParser.h"
#include "cache/Store.h"
#include "sprite/SpriteRepository.h"

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

bool isTitleSprite(
    std::string_view name
) {
    static constexpr std::array names{
        std::string_view{"logo.dat"},
        std::string_view{"titlebox.dat"},
        std::string_view{"titlebutton.dat"},
        std::string_view{"runes.dat"}
    };

    for (const std::string_view candidate : names) {
        if (candidate == name) {
            return true;
        }
    }

    return false;
}

CacheTreeNode makeRoot() {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Root;

    node.key =
        "cache";

    node.label =
        "Cache";

    return node;
}

CacheTreeNode makeIndexNode(
    eld::cache::IndexId index
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Index;

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
        if (
            archiveId == 1 &&
            *name == "title.dat"
        ) {
            node.type =
                CacheTreeNodeType::Image;
        }

        node.label =
            std::string(*name);

        node.name =
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

CacheTreeNode makeSpriteFrameNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    std::string_view groupName,
    std::uint16_t frameId
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::SpriteFrame;

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/archive/" +
        std::to_string(archiveId) +
        "/sprite/" +
        std::to_string(fileId) +
        "/frame/" +
        std::to_string(frameId);

    node.label =
        "Frame " +
        std::to_string(frameId);

    node.name =
        std::string(groupName);

    node.indexId =
        static_cast<int>(index);

    node.archiveId =
        static_cast<int>(archiveId);

    node.fileId =
        static_cast<int>(fileId);

    node.frameId =
        static_cast<int>(frameId);

    return node;
}

CacheTreeNode makeSpriteNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    std::string_view groupName,
    const eld::sprite::SpriteRepository& repository
) {
    const std::vector<std::uint16_t> frameIds =
        repository.listFrameIds(
            groupName
        );

    if (frameIds.size() == 1) {
        CacheTreeNode node =
            makeSpriteFrameNode(
                index,
                archiveId,
                file.id,
                groupName,
                frameIds.front()
            );

        node.key =
            "index/" +
            std::to_string(
                static_cast<int>(index)
            ) +
            "/archive/" +
            std::to_string(archiveId) +
            "/sprite/" +
            std::to_string(file.id);

        node.label =
            std::string(groupName);

        return node;
    }

    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Sprite;

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/archive/" +
        std::to_string(archiveId) +
        "/sprite/" +
        std::to_string(file.id);

    node.label =
        std::string(groupName) +
        " (" +
        std::to_string(frameIds.size()) +
        " frames)";

    node.name =
        std::string(groupName);

    node.indexId =
        static_cast<int>(index);

    node.archiveId =
        static_cast<int>(archiveId);

    node.fileId =
        static_cast<int>(file.id);

    for (const std::uint16_t frameId : frameIds) {
        node.children.push_back(
            makeSpriteFrameNode(
                index,
                archiveId,
                file.id,
                groupName,
                frameId
            )
        );
    }

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

    std::optional<
        eld::sprite::SpriteRepository
    > spriteRepository;

    if (entry.fileId == 1) {
        spriteRepository.emplace(
            store,
            entry.fileId
        );
    }

    for (
        const eld::archive::ArchiveFile& file :
        archive->list()
    ) {
        const std::optional<std::string_view> name =
            eld::archive::findName(
                file.nameHash
            );

        if (
            spriteRepository.has_value() &&
            name.has_value() &&
            isTitleSprite(*name)
        ) {
            node.children.push_back(
                makeSpriteNode(
                    index,
                    entry.fileId,
                    file,
                    *name,
                    *spriteRepository
                )
            );

            continue;
        }

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
