#include "LocationModelBuilder.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace eld::graphics {

namespace {

std::vector<std::uint16_t>
modelIdsForType(
    const eld::location::LocationData& definition,
    std::uint8_t modelType
) {
    const bool typed =
        std::any_of(
            definition.models.begin(),
            definition.models.end(),
            [](
                const eld::location::LocationModel& model
            ) {
                return model.type.has_value();
            }
        );


    if (typed) {
        for (
            const auto& model :
            definition.models
        ) {
            if (
                model.type.has_value() &&
                *model.type == modelType
            ) {
                return {
                    model.id
                };
            }
        }

        return {};
    }


    // Untyped model lists are classic type-10 locations.
    if (modelType != 10) {
        return {};
    }


    std::vector<std::uint16_t> ids;

    ids.reserve(
        definition.models.size()
    );


    for (
        const auto& model :
        definition.models
    ) {
        ids.push_back(
            model.id
        );
    }


    return ids;
}

eld::model::ModelData combineMeshes(
    const std::vector<
        eld::model::ModelData
    >& meshes
) {
    eld::model::ModelData result;


    std::size_t vertexCount = 0;
    std::size_t faceCount = 0;
    std::size_t mappingCount = 0;


    for (
        const auto& mesh :
        meshes
    ) {
        vertexCount +=
            mesh.vertices.size();

        faceCount +=
            mesh.faces.size();

        mappingCount +=
            mesh.textureMappings.size();
    }


    result.vertices.reserve(
        vertexCount
    );

    result.faces.reserve(
        faceCount
    );

    result.textureMappings.reserve(
        mappingCount
    );


    for (
        const auto& mesh :
        meshes
    ) {
        const auto vertexOffset =
            static_cast<std::uint32_t>(
                result.vertices.size()
            );

        const auto mappingOffset =
            static_cast<std::uint32_t>(
                result.textureMappings.size()
            );


        result.vertices.insert(
            result.vertices.end(),
            mesh.vertices.begin(),
            mesh.vertices.end()
        );


        for (
            auto mapping :
            mesh.textureMappings
        ) {
            mapping.originVertex +=
                vertexOffset;

            mapping.uVertex +=
                vertexOffset;

            mapping.vVertex +=
                vertexOffset;

            result.textureMappings.push_back(
                mapping
            );
        }


        for (
            auto face :
            mesh.faces
        ) {
            face.a += vertexOffset;
            face.b += vertexOffset;
            face.c += vertexOffset;


            if (
                face.textureMappingIndex
                    .has_value()
            ) {
                *face.textureMappingIndex +=
                    mappingOffset;
            }


            result.faces.push_back(
                face
            );
        }
    }


    return result;
}

void mirrorModel(
    eld::model::ModelData& mesh
) {
    for (
        auto& vertex :
        mesh.vertices
    ) {
        vertex.z =
            -vertex.z;
    }


    // Reflection reverses triangle winding.
    for (
        auto& face :
        mesh.faces
    ) {
        std::swap(
            face.a,
            face.c
        );
    }
}

void rotateY90(
    eld::model::ModelData& mesh
) {
    for (
        auto& vertex :
        mesh.vertices
    ) {
        const float x =
            vertex.x;

        vertex.x =
            vertex.z;

        vertex.z =
            -x;
    }
}

void recolor(
    eld::model::ModelData& mesh,
    const eld::location::LocationData& definition
) {
    for (
        const auto& recolor :
        definition.recolors
    ) {
        for (
            auto& face :
            mesh.faces
        ) {
            if (
                face.color ==
                recolor.source
            ) {
                face.color =
                    recolor.destination;
            }
        }
    }
}

void scaleAndTranslate(
    eld::model::ModelData& mesh,
    const eld::location::LocationData& definition
) {
    constexpr int BaseScale =
        128;


    for (
        auto& vertex :
        mesh.vertices
    ) {
        const int x =
            static_cast<int>(
                vertex.x
            );

        const int y =
            static_cast<int>(
                vertex.y
            );

        const int z =
            static_cast<int>(
                vertex.z
            );


        vertex.x =
            static_cast<float>(
                x *
                    static_cast<int>(
                        definition.scaleX
                    ) /
                    BaseScale +
                static_cast<int>(
                    definition.offsetX
                )
            );

        vertex.y =
            static_cast<float>(
                y *
                    static_cast<int>(
                        definition.scaleY
                    ) /
                    BaseScale +
                static_cast<int>(
                    definition.offsetY
                )
            );

        vertex.z =
            static_cast<float>(
                z *
                    static_cast<int>(
                        definition.scaleZ
                    ) /
                    BaseScale +
                static_cast<int>(
                    definition.offsetZ
                )
            );
    }
}

float sourceContourDelta(
    const eld::world::Location& location,
    float modelX,
    float modelZ
) {
    // World heights are source terrain heights * (-1 / 128).
    // Convert the classic hillskew delta back into source-model units.
    const float southwest =
        location.groundHeights[0];

    const float southeast =
        location.groundHeights[1];

    const float northeast =
        location.groundHeights[2];

    const float northwest =
        location.groundHeights[3];


    const float x =
        (modelX + 64.0f) /
        128.0f;

    const float z =
        (modelZ + 64.0f) /
        128.0f;


    const float south =
        southwest +
        (
            southeast -
            southwest
        ) *
        x;

    const float north =
        northwest +
        (
            northeast -
            northwest
        ) *
        x;

    const float height =
        south +
        (
            north -
            south
        ) *
        z;


    const float average =
        (
            southwest +
            southeast +
            northeast +
            northwest
        ) *
        0.25f;


    return
        -(height - average) *
        128.0f;
}

void contourToGround(
    eld::model::ModelData& mesh,
    const eld::world::Location& location
) {
    for (
        auto& vertex :
        mesh.vertices
    ) {
        vertex.y +=
            sourceContourDelta(
                location,
                vertex.x,
                vertex.z
            );
    }
}

eld::model::ModelData transformModel(
    eld::model::ModelData mesh,

    const eld::location::LocationData& definition,

    int modelRotation,

    const eld::world::Location& location
) {
    const bool mirrored =
        definition.rotated ^
        (modelRotation > 3);


    if (mirrored) {
        mirrorModel(
            mesh
        );
    }


    const int turns =
        modelRotation & 3;


    for (
        int turn = 0;
        turn < turns;
        ++turn
    ) {
        rotateY90(
            mesh
        );
    }


    recolor(
        mesh,
        definition
    );


    scaleAndTranslate(
        mesh,
        definition
    );


    if (
        definition.contouredGround
    ) {
        contourToGround(
            mesh,
            location
        );
    }


    return mesh;
}

}


std::size_t
LocationModelBuilder::VariantKeyHash::operator()(
    const VariantKey& key
) const {
    return
        static_cast<std::size_t>(
            key.locationId
        ) |
        (
            static_cast<std::size_t>(
                key.modelType
            ) << 16U
        ) |
        (
            static_cast<std::size_t>(
                key.modelRotation
            ) << 24U
        );
}


LocationModelBuilder::LocationModelBuilder(
    eld::model::ModelLoader& models,
    ModelSystem& modelSystem
)
    : models_(models),
      modelSystem_(modelSystem) {
}


std::optional<eld::render::ModelHandle>
LocationModelBuilder::build(
    const eld::world::Location& location,
    const eld::location::LocationData& definition,
    std::uint8_t modelType,
    int modelRotation
) {
    const VariantKey key{
        definition.id,
        modelType,
        static_cast<std::uint8_t>(
            modelRotation & 7
        )
    };


    // Contoured geometry depends on this particular
    // location's terrain heights.
    if (!definition.contouredGround) {
        const auto cached =
            variants_.find(
                key
            );

        if (
            cached !=
            variants_.end()
        ) {
            return cached->second;
        }
    }


    const auto sourceIds =
        modelIdsForType(
            definition,
            modelType
        );

    if (sourceIds.empty()) {
        return std::nullopt;
    }


    std::vector<eld::model::ModelData>
        sourceMeshes;

    sourceMeshes.reserve(
        sourceIds.size()
    );


    for (
        const auto sourceId :
        sourceIds
    ) {
        if (!models_.contains(sourceId)) {
            ++missingModels_;
            return std::nullopt;
        }

        sourceMeshes.push_back(
            models_.get(sourceId)
        );
    }


    auto mesh =
        sourceMeshes.size() == 1
            ? std::move(
                sourceMeshes.front()
            )
            : combineMeshes(
                sourceMeshes
            );


    mesh =
        transformModel(
            std::move(mesh),
            definition,
            modelRotation,
            location
        );


    const auto handle =
        modelSystem_.create(
            mesh
        );


    if (!definition.contouredGround) {
        variants_.emplace(
            key,
            handle
        );
    }


    ++createdVariants_;

    return handle;
}


std::size_t
LocationModelBuilder::createdVariants() const {
    return createdVariants_;
}


std::size_t
LocationModelBuilder::missingModels() const {
    return missingModels_;
}

}
