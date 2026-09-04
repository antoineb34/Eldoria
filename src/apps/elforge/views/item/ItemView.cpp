#include "views/item/ItemView.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>

namespace eld::elforge {

namespace {

void appendMesh(
    eld::model::Model& destination,
    eld::model::Model source
) {
    const std::uint32_t vertexOffset =
        static_cast<std::uint32_t>(destination.vertices.size());

    const std::uint32_t mappingOffset =
        static_cast<std::uint32_t>(destination.textureMappings.size());

    for (eld::model::Face& face : source.faces) {
        face.a += vertexOffset;
        face.b += vertexOffset;
        face.c += vertexOffset;

        if (face.textureMappingIndex.has_value()) {
            *face.textureMappingIndex += mappingOffset;
        }
    }

    for (eld::model::TextureMapping& mapping : source.textureMappings) {
        mapping.originVertex += vertexOffset;
        mapping.uVertex += vertexOffset;
        mapping.vVertex += vertexOffset;
    }

    destination.vertices.insert(
        destination.vertices.end(),
        source.vertices.begin(),
        source.vertices.end()
    );

    destination.faces.insert(
        destination.faces.end(),
        source.faces.begin(),
        source.faces.end()
    );

    destination.textureMappings.insert(
        destination.textureMappings.end(),
        source.textureMappings.begin(),
        source.textureMappings.end()
    );
}

void applyItemRecolors(
    eld::model::Model& mesh,
    const eld::item::Item& definition
) {
    for (eld::model::Face& face : mesh.faces) {
        for (const eld::item::ItemRecolor& recolor : definition.recolors) {
            if (face.color == recolor.source) {
                face.color = recolor.destination;
                break;
            }
        }
    }
}

void applyIdentityKitRecolors(
    eld::model::Model& mesh,
    const eld::identity_kit::IdentityKit& definition
) {
    for (eld::model::Face& face : mesh.faces) {
        for (std::size_t index = 0; index < definition.recolorSources.size(); ++index) {
            if (
                definition.recolorSources[index].has_value() &&
                definition.recolorDestinations[index].has_value() &&
                face.color == *definition.recolorSources[index]
            ) {
                face.color = *definition.recolorDestinations[index];
                break;
            }
        }
    }
}

std::string lowerCopy(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

bool containsAny(
    const std::string& text,
    std::initializer_list<const char*> needles
) {
    return std::any_of(
        needles.begin(),
        needles.end(),
        [&](const char* needle) {
            return text.find(needle) != std::string::npos;
        }
    );
}

// Identity-kit body part ids are the seven character-editor choices used by
// the classic client: head, jaw, torso, arms, hands, legs, feet. Male uses
// ids 0..6 and female uses 7..13.
enum class BobPart : std::size_t {
    Head = 0,
    Jaw = 1,
    Torso = 2,
    Arms = 3,
    Hands = 4,
    Legs = 5,
    Feet = 6
};

using HiddenBobParts = std::array<bool, 7>;

void hideWearPosition(
    HiddenBobParts& hidden,
    eld::item::ItemWearPosition position
) {
    using eld::item::ItemWearPosition;

    switch (position) {
        case ItemWearPosition::Head:
            hidden[static_cast<std::size_t>(BobPart::Head)] = true;
            break;
        case ItemWearPosition::Jaw:
            hidden[static_cast<std::size_t>(BobPart::Jaw)] = true;
            break;
        case ItemWearPosition::Torso:
            hidden[static_cast<std::size_t>(BobPart::Torso)] = true;
            break;
        case ItemWearPosition::Arms:
            hidden[static_cast<std::size_t>(BobPart::Arms)] = true;
            break;
        case ItemWearPosition::Hands:
            hidden[static_cast<std::size_t>(BobPart::Hands)] = true;
            break;
        case ItemWearPosition::Legs:
            hidden[static_cast<std::size_t>(BobPart::Legs)] = true;
            break;
        case ItemWearPosition::Feet:
            hidden[static_cast<std::size_t>(BobPart::Feet)] = true;
            break;
        default:
            break;
    }
}

HiddenBobParts hiddenBodyParts(
    const eld::item::Item& definition
) {
    HiddenBobParts hidden{};

    bool hasExactWearData = false;

    for (
        const std::optional<eld::item::ItemWearPosition>& position :
        {definition.wearPosition, definition.wearPosition2, definition.wearPosition3}
    ) {
        if (!position.has_value()) {
            continue;
        }

        hasExactWearData = true;
        hideWearPosition(hidden, *position);
    }

    if (hasExactWearData) {
        return hidden;
    }

    // The stock 317 client cache does not carry the server's wearpos fields.
    // Fall back only for body-replacing garments; weapons, shields, capes and
    // ordinary hats are intentionally left as overlays. This keeps the
    // mannequin useful without pretending the missing server metadata exists.
    const std::string name = lowerCopy(definition.name);

    if (containsAny(name, {
        "platebody", "chainbody", "body", "robe top", "shirt", "jacket"
    })) {
        hidden[static_cast<std::size_t>(BobPart::Torso)] = true;
        hidden[static_cast<std::size_t>(BobPart::Arms)] = true;
    }

    if (containsAny(name, {
        "platelegs", "legs", "trousers", "skirt", "chaps"
    })) {
        hidden[static_cast<std::size_t>(BobPart::Legs)] = true;
    }

    if (containsAny(name, {"gloves", "gauntlets", "vambraces"})) {
        hidden[static_cast<std::size_t>(BobPart::Hands)] = true;
    }

    if (containsAny(name, {"boots", "shoes"})) {
        hidden[static_cast<std::size_t>(BobPart::Feet)] = true;
    }

    if (containsAny(name, {"full helm", "full helmet", "mask"})) {
        hidden[static_cast<std::size_t>(BobPart::Head)] = true;
        hidden[static_cast<std::size_t>(BobPart::Jaw)] = true;
    }
    else if (containsAny(name, {"hood"})) {
        hidden[static_cast<std::size_t>(BobPart::Head)] = true;
    }

    return hidden;
}

const eld::identity_kit::IdentityKit* defaultIdentityKit(
    std::uint8_t bodyPartId,
    const eld::identity_kit::IdentityKitRepository& repository
) {
    for (
        std::uint16_t id :
        repository.listIds()
    ) {
        const eld::identity_kit::IdentityKit& kit =
            repository.get(id);
        if (
            kit.selectable &&
            kit.bodyPartId.has_value() &&
            *kit.bodyPartId == bodyPartId &&
            !kit.modelIds.empty()
        ) {
            return &kit;
        }
    }

    return nullptr;
}

}

std::optional<eld::model::Model>
ItemView::build(
    const eld::item::Item& definition,
    const eld::model::ModelRepository& repository
) const {
    if (!definition.inventoryModelId.has_value()) {
        return std::nullopt;
    }

    std::optional<eld::model::Model> model =
        repository.find(*definition.inventoryModelId);

    if (!model.has_value()) {
        return std::nullopt;
    }

    model->id = definition.id;

    for (eld::model::Vertex& vertex : model->vertices) {
        vertex.x =
            vertex.x * static_cast<float>(definition.scaleX) / 128.0f;
        vertex.y =
            vertex.y * static_cast<float>(definition.scaleY) / 128.0f;
        vertex.z =
            vertex.z * static_cast<float>(definition.scaleZ) / 128.0f;
    }

    applyItemRecolors(*model, definition);

    return model;
}

bool ItemView::hasEquippedModel(
    const eld::item::Item& definition,
    ItemViewGender gender
) const {
    const auto& ids =
        gender == ItemViewGender::Male
            ? definition.maleModelIds
            : definition.femaleModelIds;

    return std::any_of(
        ids.begin(),
        ids.end(),
        [](const std::optional<std::uint16_t>& id) {
            return id.has_value();
        }
    );
}

std::optional<eld::model::Model>
ItemView::buildEquipped(
    const eld::item::Item& definition,
    ItemViewGender gender,
    const eld::identity_kit::IdentityKitRepository& identityKits,
    const eld::model::ModelRepository& repository
) const {
    if (!hasEquippedModel(definition, gender)) {
        return std::nullopt;
    }

    eld::model::Model combined;
    combined.id = definition.id;

    const HiddenBobParts hidden = hiddenBodyParts(definition);
    const std::uint8_t genderOffset =
        gender == ItemViewGender::Male ? 0 : 7;

    bool foundBody = false;

    for (std::size_t part = 0; part < hidden.size(); ++part) {
        if (hidden[part]) {
            continue;
        }

        const auto* kit = defaultIdentityKit(
            static_cast<std::uint8_t>(part) + genderOffset,
            identityKits
        );

        if (kit == nullptr) {
            continue;
        }

        for (const std::uint16_t modelId : kit->modelIds) {
            std::optional<eld::model::Model> model = repository.find(modelId);

            if (!model.has_value()) {
                continue;
            }

            applyIdentityKitRecolors(*model, *kit);
            appendMesh(combined, std::move(*model));
            foundBody = true;
        }
    }

    const auto& equippedIds =
        gender == ItemViewGender::Male
            ? definition.maleModelIds
            : definition.femaleModelIds;

    const std::int8_t verticalOffset =
        gender == ItemViewGender::Male
            ? definition.maleModelOffset
            : definition.femaleModelOffset;

    bool foundEquipment = false;

    for (const std::optional<std::uint16_t>& modelId : equippedIds) {
        if (!modelId.has_value()) {
            continue;
        }

        std::optional<eld::model::Model> model = repository.find(*modelId);

        if (!model.has_value()) {
            continue;
        }

        // Matches Item.getEquippedModel(): worn models use their
        // authored size, receive only the gender-specific Y translation and
        // item recolours, and are then merged into the player appearance.
        if (verticalOffset != 0) {
            for (eld::model::Vertex& vertex : model->vertices) {
                vertex.y += static_cast<float>(verticalOffset);
            }
        }

        applyItemRecolors(*model, definition);
        appendMesh(combined, std::move(*model));
        foundEquipment = true;
    }

    if (!foundBody || !foundEquipment) {
        return std::nullopt;
    }

    return combined;
}

}
