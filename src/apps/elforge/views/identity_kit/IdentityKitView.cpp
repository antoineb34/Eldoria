#include "views/identity_kit/IdentityKitView.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace eld::elforge {

namespace {

void applyRecolors(
    eld::model::Model& mesh,
    const eld::identity_kit::IdentityKit& definition
) {
    for (eld::model::Face& face : mesh.faces) {
        for (
            std::size_t index = 0;
            index < definition.recolorSources.size();
            ++index
        ) {
            if (
                definition.recolorSources[index].has_value() &&
                definition.recolorDestinations[index].has_value() &&
                face.color == *definition.recolorSources[index]
            ) {
                face.color =
                    *definition.recolorDestinations[index];

                break;
            }
        }
    }
}

void appendMesh(
    eld::model::Model& destination,
    eld::model::Model source
) {
    const std::uint32_t vertexOffset =
        static_cast<std::uint32_t>(
            destination.vertices.size()
        );

    const std::uint32_t mappingOffset =
        static_cast<std::uint32_t>(
            destination.textureMappings.size()
        );

    for (eld::model::Face& face : source.faces) {
        face.a += vertexOffset;
        face.b += vertexOffset;
        face.c += vertexOffset;

        if (face.textureMappingIndex.has_value()) {
            *face.textureMappingIndex +=
                mappingOffset;
        }
    }

    for (
        eld::model::TextureMapping& mapping :
        source.textureMappings
    ) {
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

}

std::optional<eld::model::Model>
IdentityKitView::build(
    const eld::identity_kit::IdentityKit& definition,
    const eld::model::ModelRepository& repository
) const {
    std::vector<std::uint16_t> modelIds =
        definition.modelIds;

    if (modelIds.empty()) {
        for (
            const std::optional<std::uint16_t>& modelId :
            definition.headModelIds
        ) {
            if (modelId.has_value()) {
                modelIds.push_back(*modelId);
            }
        }
    }

    eld::model::Model combined;
    combined.id = definition.id;

    bool foundModel = false;

    for (const std::uint16_t modelId : modelIds) {
        std::optional<eld::model::Model> model =
            repository.find(modelId);

        if (!model.has_value()) {
            continue;
        }

        applyRecolors(
            *model,
            definition
        );

        appendMesh(
            combined,
            std::move(*model)
        );

        foundModel = true;
    }

    if (!foundModel) {
        return std::nullopt;
    }

    return combined;
}

}
