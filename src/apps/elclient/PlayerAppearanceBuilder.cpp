#include "PlayerAppearanceBuilder.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace eld::client {

const eld::identity_kit::IdentityKitData*
PlayerAppearanceBuilder::defaultKit(
    std::uint8_t bodyPartId,
    const eld::identity_kit::IdentityKitLoader& identityKits
) const {
    for (
        const std::uint16_t id :
        identityKits.listIds()
    ) {
        const auto& kit =
            identityKits.data(id);

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


void PlayerAppearanceBuilder::applyRecolors(
    eld::model::ModelData& model,
    const eld::identity_kit::IdentityKitData& kit
) const {
    for (
        auto& face :
        model.faces
    ) {
        for (
            std::size_t i = 0;
            i < kit.recolorSources.size();
            ++i
        ) {
            if (
                !kit.recolorSources[i].has_value() ||
                !kit.recolorDestinations[i].has_value()
            ) {
                continue;
            }

            if (
                face.color ==
                *kit.recolorSources[i]
            ) {
                face.color =
                    *kit.recolorDestinations[i];

                break;
            }
        }
    }
}


void PlayerAppearanceBuilder::appendModel(
    eld::model::ModelData& destination,
    eld::model::ModelData source
) const {
    const auto vertexOffset =
        static_cast<std::uint32_t>(
            destination.vertices.size()
        );

    const auto mappingOffset =
        static_cast<std::uint32_t>(
            destination.textureMappings.size()
        );


    for (
        auto& face :
        source.faces
    ) {
        face.a +=
            vertexOffset;

        face.b +=
            vertexOffset;

        face.c +=
            vertexOffset;

        if (
            face.textureMappingIndex
                .has_value()
        ) {
            *face.textureMappingIndex +=
                mappingOffset;
        }
    }


    for (
        auto& mapping :
        source.textureMappings
    ) {
        mapping.originVertex +=
            vertexOffset;

        mapping.uVertex +=
            vertexOffset;

        mapping.vVertex +=
            vertexOffset;
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


std::optional<eld::model::ModelData>
PlayerAppearanceBuilder::buildDefaultMale(
    const eld::identity_kit::IdentityKitLoader& identityKits,
    const eld::model::ModelLoader& models
) const {
    eld::model::ModelData combined;

    bool foundAny =
        false;


    // Classic player body:
    //
    // 0 head
    // 1 jaw
    // 2 torso
    // 3 arms
    // 4 hands
    // 5 legs
    // 6 feet
    for (
        std::uint8_t bodyPart = 0;
        bodyPart < 7;
        ++bodyPart
    ) {
        const auto* kit =
            defaultKit(
                bodyPart,
                identityKits
            );

        if (kit == nullptr) {
            continue;
        }


        for (
            const std::uint16_t modelId :
            kit->modelIds
        ) {
            const auto* source =
                models.find(modelId);

            if (source == nullptr) {
                continue;
            }

            auto part =
                *source;

            applyRecolors(
                part,
                *kit
            );

            appendModel(
                combined,
                std::move(part)
            );

            foundAny =
                true;
        }
    }


    if (!foundAny) {
        return std::nullopt;
    }

    return combined;
}

}
