#include "explorer/tree/CacheTreeBuilder.h"

#include "repositories/IdentityKitRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/NpcRepository.h"
#include "repositories/ItemRepository.h"
#include "repositories/SequenceRepository.h"
#include "repositories/SpotAnimationRepository.h"
#include "repositories/VarpRepository.h"
#include "repositories/VarbitRepository.h"
#include "repositories/ParameterRepository.h"
#include "repositories/MessageRepository.h"
#include "repositories/MessageAnimationRepository.h"
#include "map/MapRepository.h"
#include "repositories/WidgetRepository.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_set>

#include "archive/ArchiveHashes.h"
#include "archive/ArchiveParser.h"
#include "cache/Store.h"
#include "repositories/SpriteRepository.h"
#include "repositories/FloorRepository.h"

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

std::string_view getInterfaceTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0:
            return "Container";
        case 1:
            return "Unknown";
        case 2:
            return "Inventory";
        case 3:
            return "Rectangle";
        case 4:
            return "Text";
        case 5:
            return "Sprite";
        case 6:
            return "Model";
        case 7:
            return "Item List";
        case 8:
            return "Tooltip";
        default:
            return "Widget";
    }
}

CacheTreeNode makeSpriteFrameNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    std::string_view groupName,
    std::uint16_t frameId
);

CacheTreeNode makeMediaSpriteNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    std::optional<std::string_view> name,
    const eld::sprite::SpriteRepository& repository
) {
    const std::vector<std::uint16_t> frameIds =
        repository.listFrameIds(file.id);

    const std::string groupLabel =
        name.has_value()
            ? std::string(*name)
            : "Sprite File " +
                std::to_string(file.id);

    if (frameIds.size() == 1) {
        CacheTreeNode node =
            makeSpriteFrameNode(
                index,
                archiveId,
                file.id,
                groupLabel,
                frameIds.front()
            );

        node.key =
            "index/" +
            std::to_string(static_cast<int>(index)) +
            "/archive/" +
            std::to_string(archiveId) +
            "/sprite/" +
            std::to_string(file.id);

        node.label = groupLabel;

        return node;
    }

    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Sprite;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/sprite/" +
        std::to_string(file.id);

    node.label =
        groupLabel +
        " (" +
        std::to_string(frameIds.size()) +
        " frames)";

    node.name = groupLabel;
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const std::uint16_t frameId : frameIds) {
        node.children.push_back(
            makeSpriteFrameNode(
                index,
                archiveId,
                file.id,
                groupLabel,
                frameId
            )
        );
    }

    return node;
}

CacheTreeNode makeInterfaceNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::interface::Widget& definition,
    const eld::interface::WidgetRepository& repository,
    std::unordered_set<std::uint16_t>& visited
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Widget;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/interfaces/" +
        std::to_string(definition.id);

    node.label =
        std::string(getInterfaceTypeName(definition.type)) +
        " " +
        std::to_string(definition.id);

    if (!definition.text.empty()) {
        node.label +=
            " - " +
            definition.text.substr(0, 40);
    }

    node.name = "data";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    if (!visited.insert(definition.id).second) {
        return node;
    }

    for (
        const eld::interface::WidgetChild& child :
        definition.children
    ) {
        const auto* childDefinition =
            repository.find(child.id);

        if (
            childDefinition != nullptr &&
            !visited.contains(child.id)
        ) {
            node.children.push_back(
                makeInterfaceNode(
                    index,
                    archiveId,
                    fileId,
                    *childDefinition,
                    repository,
                    visited
                )
            );
        }
    }

    return node;
}

CacheTreeNode makeInterfaceGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::interface::WidgetRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/interfaces";

    node.label =
        "Interfaces (" +
        std::to_string(repository.count()) +
        " widgets)";

    node.name = "data";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    std::unordered_set<std::uint16_t> referenced;

    for (
        const auto& definition :
        repository.list()
    ) {
        for (
            const auto& child :
            definition.children
        ) {
            referenced.insert(child.id);
        }
    }

    std::unordered_set<std::uint16_t> visited;

    for (
        const auto& definition :
        repository.list()
    ) {
        if (!referenced.contains(definition.id)) {
            node.children.push_back(
                makeInterfaceNode(
                    index,
                    archiveId,
                    file.id,
                    definition,
                    repository,
                    visited
                )
            );
        }
    }

    for (
        const auto& definition :
        repository.list()
    ) {
        if (!visited.contains(definition.id)) {
            node.children.push_back(
                makeInterfaceNode(
                    index,
                    archiveId,
                    file.id,
                    definition,
                    repository,
                    visited
                )
            );
        }
    }

    return node;
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

    if (index == eld::cache::IndexId::Models) {
        node.type = CacheTreeNodeType::Model;
    }
    else if (index == eld::cache::IndexId::Animations) {
        node.type = CacheTreeNodeType::Animation;
    }
    else {
        node.type = CacheTreeNodeType::File;
    }

    node.key =
        "index/" +
        std::to_string(
            static_cast<int>(index)
        ) +
        "/file/" +
        std::to_string(entry.fileId);

    if (index == eld::cache::IndexId::Models) {
        node.label =
            "Model " +
            std::to_string(entry.fileId);
    }
    else if (index == eld::cache::IndexId::Animations) {
        node.label =
            "Animation " +
            std::to_string(entry.fileId);
    }
    else {
        node.label =
            "File " +
            std::to_string(entry.fileId);
    }

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

        if (
            archiveId == 1 &&
            (
                *name == "p11_full.dat" ||
                *name == "p12_full.dat" ||
                *name == "b12_full.dat" ||
                *name == "q8_full.dat"
            )
        ) {
            node.type =
                CacheTreeNodeType::Font;
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


template<typename Definition>
CacheTreeNode makeEmptyDefinitionNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    CacheTreeNodeType type,
    std::string_view keyName,
    std::string_view labelName,
    const Definition& definition
) {
    CacheTreeNode node;

    node.type = type;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/" +
        std::string(keyName) +
        "/" +
        std::to_string(definition.id);

    node.label =
        std::string(labelName) +
        " " +
        std::to_string(definition.id);

    node.name = std::string(keyName);
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

template<typename Repository>
CacheTreeNode makeEmptyDefinitionGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    CacheTreeNodeType childType,
    std::string_view keyName,
    std::string_view groupLabel,
    std::string_view childLabel,
    const Repository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/" +
        std::string(keyName);

    node.label =
        std::string(groupLabel) +
        " (" +
        std::to_string(repository.count()) +
        " empty definitions)";

    node.name = std::string(keyName);
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    if constexpr (
        requires {
            repository.listIds();
        }
    ) {
        for (const auto id : repository.listIds()) {
            const auto definition =
                repository.get(id);

            node.children.push_back(
                makeEmptyDefinitionNode(
                    index,
                    archiveId,
                    file.id,
                    childType,
                    keyName,
                    childLabel,
                    definition
                )
            );
        }
    }
    else {
        for (const auto& definition : repository.list()) {
            node.children.push_back(
                makeEmptyDefinitionNode(
                    index,
                    archiveId,
                    file.id,
                    childType,
                    keyName,
                    childLabel,
                    definition
                )
            );
        }
    }

    return node;
}

CacheTreeNode makeParameterNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::parameter::Parameter& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Parameter;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/param/" +
        std::to_string(definition.id);

    node.label =
        "Parameter " +
        std::to_string(definition.id);

    if (definition.type.has_value()) {
        node.label +=
            " (" +
            std::string(1, *definition.type) +
            ")";
    }

    node.name = "param";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeParameterGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::parameter::ParameterRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/param";

    node.label =
        "Parameters (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "param";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::parameter::Parameter definition =
            repository.get(id);
        node.children.push_back(
            makeParameterNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

template<typename Definition>
CacheTreeNode makeVariableNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    CacheTreeNodeType type,
    std::string_view keyName,
    std::string_view labelName,
    const Definition& definition
) {
    CacheTreeNode node;

    node.type = type;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/" +
        std::string(keyName) +
        "/" +
        std::to_string(definition.id);

    node.label =
        std::string(labelName) +
        " " +
        std::to_string(definition.id);

    if (!definition.name.empty()) {
        node.label += " - " + definition.name;
    }

    node.name = std::string(keyName);
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

template<typename Repository>
CacheTreeNode makeVariableGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    CacheTreeNodeType childType,
    std::string_view keyName,
    std::string_view groupLabel,
    std::string_view childLabel,
    const Repository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/" +
        std::string(keyName);

    node.label =
        std::string(groupLabel) +
        " (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = std::string(keyName);
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const auto definition =
            repository.get(id);
        node.children.push_back(
            makeVariableNode(
                index,
                archiveId,
                file.id,
                childType,
                keyName,
                childLabel,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeSpotAnimationNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::spot_animation::SpotAnimation& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::SpotAnimation;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/spotanim/" +
        std::to_string(definition.id);

    node.label =
        "Spot Animation " +
        std::to_string(definition.id);

    node.name = "spotanim";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeSpotAnimationGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::spot_animation::SpotAnimationRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/spotanim";

    node.label =
        "Spot Animations (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "spotanim";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::spot_animation::SpotAnimation definition =
            repository.get(id);
        node.children.push_back(
            makeSpotAnimationNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeSequenceNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::sequence::Sequence& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Sequence;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/seq/" +
        std::to_string(definition.id);

    node.label =
        "Sequence " +
        std::to_string(definition.id) +
        " (" +
        std::to_string(definition.frames.size()) +
        " frames)";

    node.name = "seq";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeSequenceGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::sequence::SequenceRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/seq";

    node.label =
        "Sequences (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "seq";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::sequence::Sequence definition =
            repository.get(id);
        node.children.push_back(
            makeSequenceNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeItemNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::item::Item& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Item;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/obj/" +
        std::to_string(definition.id);

    node.label =
        "Item " +
        std::to_string(definition.id);

    if (
        !definition.name.empty() &&
        definition.name != "null"
    ) {
        node.label +=
            " - " +
            definition.name;
    }

    node.name = "obj";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeItemGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::item::ItemRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/obj";

    node.label =
        "Items (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "obj";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::item::Item definition =
            repository.get(id);
        node.children.push_back(
            makeItemNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeNpcNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::npc::Npc& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Npc;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/npc/" +
        std::to_string(definition.id);

    node.label =
        "NPC " +
        std::to_string(definition.id);

    if (
        !definition.name.empty() &&
        definition.name != "null"
    ) {
        node.label +=
            " - " +
            definition.name;
    }

    node.name = "npc";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeNpcGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::npc::NpcRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/npc";

    node.label =
        "NPCs (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "npc";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::npc::Npc definition =
            repository.get(id);
        node.children.push_back(
            makeNpcNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeLocationNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::location::Location& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Location;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/loc/" +
        std::to_string(definition.id);

    node.label =
        "Location " +
        std::to_string(definition.id);

    if (
        !definition.name.empty() &&
        definition.name != "null"
    ) {
        node.label +=
            " - " +
            definition.name;
    }

    node.name = "loc";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeLocationGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::location::LocationRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/loc";

    node.label =
        "Locations (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "loc";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (const auto id : repository.listIds()) {
        const eld::location::Location definition =
            repository.get(id);
        node.children.push_back(
            makeLocationNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeIdentityKitNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::identity_kit::IdentityKit& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::IdentityKit;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/idk/" +
        std::to_string(definition.id);

    node.label =
        "Identity Kit " +
        std::to_string(definition.id);

    if (definition.bodyPartId.has_value()) {
        node.label +=
            " (body part " +
            std::to_string(*definition.bodyPartId) +
            ")";
    }

    node.name = "idk";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeIdentityKitGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::identity_kit::IdentityKitRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/idk";

    node.label =
        "Identity Kits (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "idk";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (
        std::uint16_t id :
        repository.listIds()
    ) {
        const eld::identity_kit::IdentityKit& definition =
            repository.get(id);
        node.children.push_back(
            makeIdentityKitNode(
                index,
                archiveId,
                file.id,
                definition
            )
        );
    }

    return node;
}

CacheTreeNode makeFloorNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::floor::Floor& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Floor;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/flo/" +
        std::to_string(definition.id);

    node.label =
        "Floor " +
        std::to_string(definition.id);

    if (!definition.name.empty()) {
        node.label +=
            " - " +
            definition.name;
    }

    node.name = "flo";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(fileId);
    node.definitionId =
        static_cast<int>(definition.id);

    return node;
}

CacheTreeNode makeFloorGroupNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    const eld::archive::ArchiveFile& file,
    const eld::floor::FloorRepository& repository
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::DefinitionGroup;

    node.key =
        "index/" +
        std::to_string(static_cast<int>(index)) +
        "/archive/" +
        std::to_string(archiveId) +
        "/definitions/flo";

    node.label =
        "Floors (" +
        std::to_string(repository.count()) +
        " definitions)";

    node.name = "flo";
    node.indexId = static_cast<int>(index);
    node.archiveId = static_cast<int>(archiveId);
    node.fileId = static_cast<int>(file.id);

    for (
        std::uint16_t id :
        repository.listIds()
    ) {
        const eld::floor::Floor& floor =
            repository.get(id);

        node.children.push_back(
            makeFloorNode(
                index,
                archiveId,
                file.id,
                floor
            )
        );
    }

    return node;
}

std::optional<CacheTreeNode> makeArchiveNode(
    eld::cache::IndexId index,
    const eld::cache::Cache& cache,
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

    if (
        entry.fileId == 1 ||
        entry.fileId == 4
    ) {
        spriteRepository.emplace(
            cache,
            entry.fileId
        );
    }

    std::optional<
        eld::interface::WidgetRepository
    > widgetRepository;

    if (entry.fileId == 3) {
        widgetRepository.emplace(
            cache
        );
    }

    std::optional<
        eld::floor::FloorRepository
    > floorRepository;

    std::optional<
        eld::identity_kit::IdentityKitRepository
    > identityKitRepository;

    std::optional<
        eld::location::LocationRepository
    > locationRepository;

    std::optional<
        eld::npc::NpcRepository
    > npcRepository;

    std::optional<
        eld::item::ItemRepository
    > itemRepository;

    std::optional<
        eld::sequence::SequenceRepository
    > sequenceRepository;

    std::optional<
        eld::spot_animation::SpotAnimationRepository
    > spotAnimationRepository;

    std::optional<
        eld::varp::VarpRepository
    > varpRepository;

    std::optional<
        eld::varbit::VarbitRepository
    > varbitRepository;

    std::optional<
        eld::parameter::ParameterRepository
    > parameterRepository;

    std::optional<
        eld::message::MessageRepository
    > messageRepository;

    std::optional<
        eld::message_animation::MessageAnimationRepository
    > messageAnimationRepository;

    if (entry.fileId == 2) {

        floorRepository.emplace(
            cache
        );

        identityKitRepository.emplace(
            cache
        );

        locationRepository.emplace(
            cache
        );

        npcRepository.emplace(
            cache
        );

        itemRepository.emplace(
            cache
        );

        sequenceRepository.emplace(
            cache
        );

        spotAnimationRepository.emplace(
            cache
        );

        varpRepository.emplace(
            cache
        );

        varbitRepository.emplace(
            cache
        );

        parameterRepository.emplace(
            cache
        );

        messageRepository.emplace(
            cache
        );

        messageAnimationRepository.emplace(
            cache
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
            entry.fileId == 4 &&
            spriteRepository.has_value() &&
            (
                !name.has_value() ||
                *name != "index.dat"
            )
        ) {
            const std::vector<std::uint16_t> frameIds =
                spriteRepository->listFrameIds(
                    file.id
                );

            if (!frameIds.empty()) {
                node.children.push_back(
                    makeMediaSpriteNode(
                        index,
                        entry.fileId,
                        file,
                        name,
                        *spriteRepository
                    )
                );

                continue;
            }
        }

        if (
            widgetRepository.has_value() &&
            name.has_value() &&
            *name == "data"
        ) {
            node.children.push_back(
                makeInterfaceGroupNode(
                    index,
                    entry.fileId,
                    file,
                    *widgetRepository
                )
            );

            continue;
        }

        if (
            messageAnimationRepository.has_value() &&
            name.has_value() &&
            *name == "mesanim.dat"
        ) {
            node.children.push_back(
                makeEmptyDefinitionGroupNode(
                    index,
                    entry.fileId,
                    file,
                    CacheTreeNodeType::MessageAnimation,
                    "mesanim",
                    "Message Animations",
                    "Message Animation",
                    *messageAnimationRepository
                )
            );

            continue;
        }

        if (
            name.has_value() &&
            *name == "mesanim.idx"
        ) {
            continue;
        }

        if (
            messageRepository.has_value() &&
            name.has_value() &&
            *name == "mes.dat"
        ) {
            node.children.push_back(
                makeEmptyDefinitionGroupNode(
                    index,
                    entry.fileId,
                    file,
                    CacheTreeNodeType::Message,
                    "mes",
                    "Messages",
                    "Message",
                    *messageRepository
                )
            );

            continue;
        }

        if (
            name.has_value() &&
            *name == "mes.idx"
        ) {
            continue;
        }

        if (
            parameterRepository.has_value() &&
            name.has_value() &&
            *name == "param.dat"
        ) {
            node.children.push_back(
                makeParameterGroupNode(
                    index,
                    entry.fileId,
                    file,
                    *parameterRepository
                )
            );

            continue;
        }

        if (
            name.has_value() &&
            *name == "param.idx"
        ) {
            continue;
        }

        if (
            varpRepository.has_value() &&
            name.has_value() &&
            *name == "varp.dat"
        ) {
            node.children.push_back(
                makeVariableGroupNode(
                    index,
                    entry.fileId,
                    file,
                    CacheTreeNodeType::Varp,
                    "varp",
                    "Varps",
                    "Varp",
                    *varpRepository
                )
            );

            continue;
        }

        if (
            name.has_value() &&
            *name == "varp.idx"
        ) {
            continue;
        }

        if (
            varbitRepository.has_value() &&
            name.has_value() &&
            *name == "varbit.dat"
        ) {
            node.children.push_back(
                makeVariableGroupNode(
                    index,
                    entry.fileId,
                    file,
                    CacheTreeNodeType::Varbit,
                    "varbit",
                    "Varbits",
                    "Varbit",
                    *varbitRepository
                )
            );

            continue;
        }

        if (
            name.has_value() &&
            *name == "varbit.idx"
        ) {
            continue;
        }

        if (
            spotAnimationRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "spotanim.dat") {
                node.children.push_back(
                    makeSpotAnimationGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *spotAnimationRepository
                    )
                );

                continue;
            }

            if (*name == "spotanim.idx") {
                continue;
            }
        }

        if (
            sequenceRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "seq.dat") {
                node.children.push_back(
                    makeSequenceGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *sequenceRepository
                    )
                );

                continue;
            }

            if (*name == "seq.idx") {
                continue;
            }
        }

        if (
            itemRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "obj.dat") {
                node.children.push_back(
                    makeItemGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *itemRepository
                    )
                );

                continue;
            }

            if (*name == "obj.idx") {
                continue;
            }
        }

        if (
            npcRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "npc.dat") {
                node.children.push_back(
                    makeNpcGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *npcRepository
                    )
                );

                continue;
            }

            if (*name == "npc.idx") {
                continue;
            }
        }

        if (
            locationRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "loc.dat") {
                node.children.push_back(
                    makeLocationGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *locationRepository
                    )
                );

                continue;
            }

            if (*name == "loc.idx") {
                continue;
            }
        }

        if (
            identityKitRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "idk.dat") {
                node.children.push_back(
                    makeIdentityKitGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *identityKitRepository
                    )
                );

                continue;
            }

            if (*name == "idk.idx") {
                continue;
            }
        }

        if (
            floorRepository.has_value() &&
            name.has_value()
        ) {
            if (*name == "flo.dat") {
                node.children.push_back(
                    makeFloorGroupNode(
                        index,
                        entry.fileId,
                        file,
                        *floorRepository
                    )
                );

                continue;
            }

            if (*name == "flo.idx") {
                continue;
            }
        }

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


CacheTreeNode makeMidiNode(
    const eld::cache::FileEntry& entry
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::Midi;

    node.key =
        "index/3/midi/" +
        std::to_string(entry.fileId);

    node.label =
        "MIDI " +
        std::to_string(entry.fileId);

    node.name = "midi";
    node.indexId =
        static_cast<int>(
            eld::cache::IndexId::Midi
        );
    node.fileId =
        static_cast<int>(entry.fileId);

    return node;
}

CacheTreeNode makeMapRegionNode(
    const eld::map::MapIndexEntry& entry
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::MapRegion;

    node.key =
        "index/4/region/" +
        std::to_string(entry.regionId);

    node.label =
        "Region " +
        std::to_string(entry.regionId) +
        " (" +
        std::to_string(entry.regionX()) +
        "," +
        std::to_string(entry.regionY()) +
        ")";

    node.indexId =
        static_cast<int>(
            eld::cache::IndexId::Maps
        );

    node.regionId =
        static_cast<int>(entry.regionId);

    node.terrainFileId =
        static_cast<int>(entry.terrainFileId);

    node.locationFileId =
        static_cast<int>(entry.locationFileId);

    CacheTreeNode terrain;
    terrain.type = CacheTreeNodeType::File;
    terrain.key =
        node.key + "/terrain/" +
        std::to_string(entry.terrainFileId);
    terrain.label =
        "Terrain file " +
        std::to_string(entry.terrainFileId);
    terrain.indexId =
        static_cast<int>(
            eld::cache::IndexId::Maps
        );
    terrain.fileId =
        static_cast<int>(entry.terrainFileId);
    terrain.regionId = node.regionId;
    terrain.terrainFileId = node.terrainFileId;
    terrain.locationFileId = node.locationFileId;

    CacheTreeNode objects;
    objects.type = CacheTreeNodeType::File;
    objects.key =
        node.key + "/objects/" +
        std::to_string(entry.locationFileId);
    objects.label =
        "Object file " +
        std::to_string(entry.locationFileId);
    objects.indexId =
        static_cast<int>(
            eld::cache::IndexId::Maps
        );
    objects.fileId =
        static_cast<int>(entry.locationFileId);
    objects.regionId = node.regionId;
    objects.terrainFileId = node.terrainFileId;
    objects.locationFileId = node.locationFileId;

    node.children.push_back(
        std::move(terrain)
    );

    node.children.push_back(
        std::move(objects)
    );

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

    if (
        index.id ==
        eld::cache::IndexId::Midi
    ) {
        const eld::cache::Store store =
            cache.open(index.id);

        const std::vector<eld::cache::FileEntry> entries =
            store.list();

        for (const eld::cache::FileEntry& entry : entries) {
            indexNode.children.push_back(
                makeMidiNode(entry)
            );
        }

        root.children.push_back(
            std::move(indexNode)
        );

        return;
    }

    if (
        index.id ==
        eld::cache::IndexId::Maps
    ) {
        const eld::map::MapRepository loader(cache);

        for (
            const eld::map::MapIndexEntry& entry :
            loader.entries()
        ) {
            indexNode.children.push_back(
                makeMapRegionNode(entry)
            );
        }

        root.children.push_back(
            std::move(indexNode)
        );

        return;
    }

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
                    cache,
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


bool populateTextureArchive(
    CacheTreeNode& node,
    const std::vector<std::uint16_t>& textureIds
) {
    constexpr int ConfigIndexId = 0;
    constexpr int TextureArchiveId = 6;

    if (
        node.type == CacheTreeNodeType::Archive &&
        node.indexId == ConfigIndexId &&
        node.archiveId == TextureArchiveId
    ) {
        node.label =
            "Archive 6 - Textures";

        node.children.clear();

        node.children.reserve(
            textureIds.size()
        );

        for (
            const std::uint16_t textureId :
            textureIds
        ) {
            CacheTreeNode textureNode;

            textureNode.type =
                CacheTreeNodeType::Texture;

            textureNode.label =
                "Texture " +
                std::to_string(
                    textureId
                );

            textureNode.key =
                node.key +
                "/texture/" +
                std::to_string(
                    textureId
                );

            textureNode.indexId =
                node.indexId;

            textureNode.archiveId =
                node.archiveId;

            textureNode.fileId =
                static_cast<int>(
                    textureId
                );

            node.children.push_back(
                std::move(
                    textureNode
                )
            );
        }

        return true;
    }

    for (
        CacheTreeNode& child :
        node.children
    ) {
        if (
            populateTextureArchive(
                child,
                textureIds
            )
        ) {
            return true;
        }
    }

    return false;
}

}

CacheTreeNode CacheTreeBuilder::build(
    const eld::cache::Cache& cache,
    const std::vector<std::uint16_t>& textureIds
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

    populateTextureArchive(
        root,
        textureIds
    );

    return root;
}

}
