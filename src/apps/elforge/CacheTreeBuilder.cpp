#include "CacheTreeBuilder.h"

#include "definition/idk/IdentityKitRepository.h"
#include "definition/location/LocationRepository.h"
#include "definition/npc/NpcRepository.h"
#include "definition/item/ItemRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "definition/varp/VarpRepository.h"
#include "definition/varbit/VarbitRepository.h"
#include "definition/parameter/ParameterRepository.h"
#include "definition/message/MessageRepository.h"
#include "definition/message_animation/MessageAnimationRepository.h"
#include "map/MapLoader.h"
#include "interface/InterfaceRepository.h"

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
#include "sprite/SpriteRepository.h"
#include "definition/DefinitionRepository.h"
#include "definition/floor/FloorRepository.h"

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
    const eld::interface::InterfaceWidget& definition,
    const eld::interface::InterfaceRepository& repository,
    std::unordered_set<std::uint16_t>& visited
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::InterfaceDefinition;

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
        const eld::interface::InterfaceFileChild& child :
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
    const eld::interface::InterfaceRepository& repository
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

    return node;
}

CacheTreeNode makeParameterNode(
    eld::cache::IndexId index,
    std::uint16_t archiveId,
    std::uint16_t fileId,
    const eld::definition::ParameterDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::ParameterDefinition;

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
    const eld::definition::ParameterRepository& repository
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

    for (
        const eld::definition::ParameterDefinition& definition :
        repository.list()
    ) {
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

    for (const auto& definition : repository.list()) {
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
    const eld::definition::SpotAnimationDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::SpotAnimationDefinition;

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
    const eld::definition::SpotAnimationRepository& repository
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

    for (
        const eld::definition::SpotAnimationDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::SequenceDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::SequenceDefinition;

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
    const eld::definition::SequenceRepository& repository
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

    for (
        const eld::definition::SequenceDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::ItemDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::ItemDefinition;

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
    const eld::definition::ItemRepository& repository
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

    for (
        const eld::definition::ItemDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::NpcDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::NpcDefinition;

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
    const eld::definition::NpcRepository& repository
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

    for (
        const eld::definition::NpcDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::LocationDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::LocationDefinition;

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
    const eld::definition::LocationRepository& repository
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

    for (
        const eld::definition::LocationDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::IdentityKitDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::IdentityKitDefinition;

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
    const eld::definition::IdentityKitRepository& repository
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
        const eld::definition::IdentityKitDefinition& definition :
        repository.list()
    ) {
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
    const eld::definition::FloorDefinition& definition
) {
    CacheTreeNode node;

    node.type =
        CacheTreeNodeType::FloorDefinition;

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
    const eld::definition::FloorRepository& repository
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
        const eld::definition::FloorDefinition& definition :
        repository.list()
    ) {
        node.children.push_back(
            makeFloorNode(
                index,
                archiveId,
                file.id,
                definition
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

    if (
        entry.fileId == 1 ||
        entry.fileId == 4
    ) {
        spriteRepository.emplace(
            store,
            entry.fileId
        );
    }

    std::optional<
        eld::interface::InterfaceRepository
    > interfaceRepository;

    if (entry.fileId == 3) {
        interfaceRepository.emplace(
            store,
            entry.fileId
        );
    }

    std::optional<
        eld::definition::DefinitionRepository
    > definitionRepository;

    std::optional<
        eld::definition::FloorRepository
    > floorRepository;

    std::optional<
        eld::definition::IdentityKitRepository
    > identityKitRepository;

    std::optional<
        eld::definition::LocationRepository
    > locationRepository;

    std::optional<
        eld::definition::NpcRepository
    > npcRepository;

    std::optional<
        eld::definition::ItemRepository
    > itemRepository;

    std::optional<
        eld::definition::SequenceRepository
    > sequenceRepository;

    std::optional<
        eld::definition::SpotAnimationRepository
    > spotAnimationRepository;

    std::optional<
        eld::definition::VarpRepository
    > varpRepository;

    std::optional<
        eld::definition::VarbitRepository
    > varbitRepository;

    std::optional<
        eld::definition::ParameterRepository
    > parameterRepository;

    std::optional<
        eld::definition::MessageRepository
    > messageRepository;

    std::optional<
        eld::definition::MessageAnimationRepository
    > messageAnimationRepository;

    if (entry.fileId == 2) {
        definitionRepository.emplace(
            store,
            entry.fileId
        );

        floorRepository.emplace(
            definitionRepository->get(
                "flo"
            )
        );

        identityKitRepository.emplace(
            definitionRepository->get(
                "idk"
            )
        );

        locationRepository.emplace(
            definitionRepository->get(
                "loc"
            )
        );

        npcRepository.emplace(
            definitionRepository->get(
                "npc"
            )
        );

        itemRepository.emplace(
            definitionRepository->get(
                "obj"
            )
        );

        sequenceRepository.emplace(
            definitionRepository->get(
                "seq"
            )
        );

        spotAnimationRepository.emplace(
            definitionRepository->get(
                "spotanim"
            )
        );

        varpRepository.emplace(
            definitionRepository->get("varp")
        );

        varbitRepository.emplace(
            definitionRepository->get("varbit")
        );

        parameterRepository.emplace(
            definitionRepository->get("param")
        );

        messageRepository.emplace(
            definitionRepository->get("mes")
        );

        messageAnimationRepository.emplace(
            definitionRepository->get("mesanim")
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
            interfaceRepository.has_value() &&
            name.has_value() &&
            *name == "data"
        ) {
            node.children.push_back(
                makeInterfaceGroupNode(
                    index,
                    entry.fileId,
                    file,
                    *interfaceRepository
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
                    CacheTreeNodeType::MessageAnimationDefinition,
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
                    CacheTreeNodeType::MessageDefinition,
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
                    CacheTreeNodeType::VarpDefinition,
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
                    CacheTreeNodeType::VarbitDefinition,
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

    node.objectFileId =
        static_cast<int>(entry.objectFileId);

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
    terrain.objectFileId = node.objectFileId;

    CacheTreeNode objects;
    objects.type = CacheTreeNodeType::File;
    objects.key =
        node.key + "/objects/" +
        std::to_string(entry.objectFileId);
    objects.label =
        "Object file " +
        std::to_string(entry.objectFileId);
    objects.indexId =
        static_cast<int>(
            eld::cache::IndexId::Maps
        );
    objects.fileId =
        static_cast<int>(entry.objectFileId);
    objects.regionId = node.regionId;
    objects.terrainFileId = node.terrainFileId;
    objects.objectFileId = node.objectFileId;

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
        eld::cache::IndexId::Maps
    ) {
        const eld::map::MapLoader loader(cache);

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
