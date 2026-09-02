#include "SceneLocationModelBuilder.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eld::render::map {
namespace {

struct VariantKey {
    std::uint16_t locationId = 0;
    std::uint8_t modelType = 0;
    std::uint8_t modelRotation = 0;

    bool operator==(const VariantKey&) const = default;
};

struct VariantKeyHash {
    std::size_t operator()(const VariantKey& key) const {
        return
            static_cast<std::size_t>(key.locationId) |
            (static_cast<std::size_t>(key.modelType) << 16U) |
            (static_cast<std::size_t>(key.modelRotation) << 24U);
    }
};

std::vector<std::uint16_t> modelIdsForType(
    const eld::definition::LocationDefinition& definition,
    std::uint8_t modelType
) {
    const bool typed =
        std::any_of(
            definition.models.begin(),
            definition.models.end(),
            [](const eld::definition::LocationModel& model) {
                return model.type.has_value();
            }
        );

    if (typed) {
        for (
            const eld::definition::LocationModel& model :
            definition.models
        ) {
            if (
                model.type.has_value() &&
                *model.type == modelType
            ) {
                // The classic LocType path selects the first shape entry.
                return {model.id};
            }
        }

        return {};
    }

    // Opcode-5 style untyped model lists represent centrepiece/type-10 locs.
    // Keep support for them even though the older May-2004 LocType source only
    // carried the typed shape list.
    if (modelType != 10) {
        return {};
    }

    std::vector<std::uint16_t> ids;
    ids.reserve(definition.models.size());

    for (
        const eld::definition::LocationModel& model :
        definition.models
    ) {
        ids.push_back(model.id);
    }

    return ids;
}

eld::model::Model combineMeshes(
    const std::vector<eld::model::Model>& meshes
) {
    eld::model::Model result;

    std::size_t vertexCount = 0;
    std::size_t faceCount = 0;
    std::size_t mappingCount = 0;

    for (const eld::model::Model& mesh : meshes) {
        vertexCount += mesh.vertices.size();
        faceCount += mesh.faces.size();
        mappingCount += mesh.textureMappings.size();
    }

    result.vertices.reserve(vertexCount);
    result.faces.reserve(faceCount);
    result.textureMappings.reserve(mappingCount);

    for (const eld::model::Model& mesh : meshes) {
        const std::uint32_t vertexOffset =
            static_cast<std::uint32_t>(result.vertices.size());
        const std::uint32_t mappingOffset =
            static_cast<std::uint32_t>(result.textureMappings.size());

        result.vertices.insert(
            result.vertices.end(),
            mesh.vertices.begin(),
            mesh.vertices.end()
        );

        for (eld::model::TextureMapping mapping : mesh.textureMappings) {
            mapping.originVertex += vertexOffset;
            mapping.uVertex += vertexOffset;
            mapping.vVertex += vertexOffset;
            result.textureMappings.push_back(mapping);
        }

        for (eld::model::Face face : mesh.faces) {
            face.a += vertexOffset;
            face.b += vertexOffset;
            face.c += vertexOffset;

            if (face.textureMappingIndex.has_value()) {
                *face.textureMappingIndex += mappingOffset;
            }

            result.faces.push_back(face);
        }
    }

    return result;
}

void rotateY90(eld::model::Model& mesh) {
    for (eld::model::Vertex& vertex : mesh.vertices) {
        const float oldX = vertex.x;
        vertex.x = vertex.z;
        vertex.z = -oldX;
    }
}

void rotateY180Mirrored(eld::model::Model& mesh) {
    for (eld::model::Vertex& vertex : mesh.vertices) {
        vertex.z = -vertex.z;
    }

    // Model::rotateY180() swaps face winding after the Z reflection.
    for (eld::model::Face& face : mesh.faces) {
        std::swap(face.a, face.c);
    }
}

void recolor(
    eld::model::Model& mesh,
    const eld::definition::LocationDefinition& definition
) {
    for (
        const eld::definition::LocationRecolor& recolor :
        definition.recolors
    ) {
        for (eld::model::Face& face : mesh.faces) {
            if (face.color == recolor.source) {
                face.color = recolor.destination;
            }
        }
    }
}

void scaleAndTranslate(
    eld::model::Model& mesh,
    const eld::definition::LocationDefinition& definition
) {
    constexpr int BaseScale = 128;

    for (eld::model::Vertex& vertex : mesh.vertices) {
        // Cache vertices are integral even though Model stores floats. The
        // classic Model scaler performs integer arithmetic, including its
        // truncate-toward-zero division by 128.
        const int x = static_cast<int>(vertex.x);
        const int y = static_cast<int>(vertex.y);
        const int z = static_cast<int>(vertex.z);

        vertex.x = static_cast<float>(
            x * static_cast<int>(definition.scaleX) / BaseScale +
            static_cast<int>(definition.offsetX)
        );
        vertex.y = static_cast<float>(
            y * static_cast<int>(definition.scaleY) / BaseScale +
            static_cast<int>(definition.offsetY)
        );
        vertex.z = static_cast<float>(
            z * static_cast<int>(definition.scaleZ) / BaseScale +
            static_cast<int>(definition.offsetZ)
        );
    }
}

SceneLocationModelPart standardPart(
    std::size_t variantIndex,
    int sceneYaw = 0
) {
    return SceneLocationModelPart{
        .variantIndex = variantIndex,
        .sceneYaw = sceneYaw,
        .drawMode = SceneLocationDrawMode::Standard
    };
}

}

eld::model::Model SceneLocationModelBuilder::transformModel(
    eld::model::Model mesh,
    const eld::definition::LocationDefinition& definition,
    int modelRotation
) {
    // LocType::getModel uses rotation > 3 only as the mirror selector; its
    // repeated quarter-turn loop effectively keeps rotation modulo four.
    const bool mirrored =
        definition.rotated ^ (modelRotation > 3);

    if (mirrored) {
        rotateY180Mirrored(mesh);
    }

    const int quarterTurns = modelRotation & 3;
    for (int turn = 0; turn < quarterTurns; ++turn) {
        rotateY90(mesh);
    }

    recolor(mesh, definition);
    scaleAndTranslate(mesh, definition);

    return mesh;
}

SceneLocationModelBuildResult SceneLocationModelBuilder::build(
    const std::vector<SceneLocationPlacement>& placements,
    const eld::definition::LocationRepository& locations,
    const eld::model::ModelRepository& models
) const {
    SceneLocationModelBuildResult result;
    result.instances.reserve(placements.size());
    result.stats.placements = placements.size();

    std::unordered_map<VariantKey, std::size_t, VariantKeyHash>
        variantCache;

    const auto resolveVariant = [&]
        (
            const eld::definition::LocationDefinition& definition,
            std::uint8_t modelType,
            int modelRotation
        ) -> std::optional<std::size_t> {
        const VariantKey key{
            definition.id,
            modelType,
            static_cast<std::uint8_t>(modelRotation & 7)
        };

        const auto existing = variantCache.find(key);
        if (existing != variantCache.end()) {
            return existing->second;
        }

        const std::vector<std::uint16_t> sourceIds =
            modelIdsForType(definition, modelType);

        if (sourceIds.empty()) {
            return std::nullopt;
        }

        std::vector<eld::model::Model> sourceMeshes;
        sourceMeshes.reserve(sourceIds.size());

        for (const std::uint16_t sourceId : sourceIds) {
            if (!models.contains(sourceId)) {
                ++result.stats.missingModelFiles;
                return std::nullopt;
            }

            sourceMeshes.push_back(
                models.get(sourceId)
            );
        }

        eld::model::Model mesh =
            sourceMeshes.size() == 1
                ? std::move(sourceMeshes.front())
                : combineMeshes(sourceMeshes);

        mesh = transformModel(
            std::move(mesh),
            definition,
            modelRotation
        );

        const std::size_t index = result.variants.size();
        result.variants.push_back({
            .locationId = definition.id,
            .modelType = modelType,
            .modelRotation = static_cast<std::uint8_t>(modelRotation & 7),
            .mesh = std::move(mesh)
        });
        variantCache.emplace(key, index);
        return index;
    };

    for (const SceneLocationPlacement& placement : placements) {
        const eld::definition::LocationDefinition* definition =
            locations.find(placement.id);

        if (definition == nullptr) {
            ++result.stats.missingDefinitions;
            continue;
        }

        SceneLocationModelInstance instance;
        instance.id = placement.id;
        instance.shape = placement.shape;
        instance.rotation = placement.rotation;
        instance.scenePlane = placement.scenePlane;
        instance.sceneX = placement.sceneX;
        instance.sceneY = placement.sceneY;
        instance.sceneZ = placement.sceneZ;
        instance.cornerHeights = placement.cornerHeights;
        instance.contouredGround = definition->contouredGround;
        instance.animated = definition->animationId.has_value();

        if (instance.contouredGround) {
            ++result.stats.contouredGround;
        }
        if (instance.animated) {
            ++result.stats.animated;
        }

        const int rotation = static_cast<int>(placement.rotation);
        const std::uint8_t shape = placement.shape;

        if (shape == 2) {
            const std::uint8_t nextRotation =
                static_cast<std::uint8_t>((placement.rotation + 1u) & 3u);

            const std::optional<std::size_t> first =
                resolveVariant(*definition, 2, rotation + 4);
            const std::optional<std::size_t> second =
                resolveVariant(*definition, 2, nextRotation);

            if (first.has_value()) {
                instance.parts.push_back(standardPart(*first));
            }
            if (second.has_value()) {
                instance.parts.push_back(standardPart(*second));
            }
        }
        else if (shape >= 4 && shape <= 8) {
            // All wall-decoration shapes request model shape 4 at rotation 0;
            // their facing is a World3D draw-time transform.
            const std::optional<std::size_t> variant =
                resolveVariant(*definition, 4, 0);

            if (variant.has_value()) {
                if (shape == 4 || shape == 5) {
                    instance.parts.push_back(
                        standardPart(
                            *variant,
                            rotation * QuarterTurn
                        )
                    );
                }
                else if (shape == 6) {
                    instance.parts.push_back({
                        .variantIndex = *variant,
                        .sceneYaw = 0,
                        .drawMode =
                            SceneLocationDrawMode::WallDecorationInset
                    });
                }
                else if (shape == 7) {
                    instance.parts.push_back({
                        .variantIndex = *variant,
                        .sceneYaw = 0,
                        .drawMode =
                            SceneLocationDrawMode::WallDecorationOutset
                    });
                }
                else {
                    instance.parts.push_back({
                        .variantIndex = *variant,
                        .sceneYaw = 0,
                        .drawMode =
                            SceneLocationDrawMode::WallDecorationDiagonalBoth
                    });
                }
            }
        }
        else {
            const std::uint8_t modelType =
                SceneLocationBuilder::normalizedModelType(shape);
            const std::optional<std::size_t> variant =
                resolveVariant(*definition, modelType, rotation);

            if (variant.has_value()) {
                instance.parts.push_back(
                    standardPart(
                        *variant,
                        shape == 11 ? EighthTurn : 0
                    )
                );
            }
        }

        if (instance.parts.empty()) {
            ++result.stats.missingShapeModels;
            continue;
        }

        result.stats.parts += instance.parts.size();
        result.instances.push_back(std::move(instance));
    }

    result.stats.instances = result.instances.size();
    result.stats.variants = result.variants.size();
    return result;
}

}
