#include "LocationBatchBuilder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "math/Vec4.h"
#include "render/scene/Transform.h"

namespace eld::graphics {

namespace {

constexpr float ChunkSize =
    16.0f;

constexpr int ChunksPerAxis =
    4;


struct BatchBucket {
    eld::render::RenderMaterial
        material;

    float depthBias = 0.0f;

    std::vector<eld::render::RenderVertex>
        vertices;

    std::vector<std::uint32_t>
        indices;
};


struct ChunkBatch {
    std::vector<BatchBucket>
        buckets;
};


bool sameColor(
    const eld::math::Vec4& a,
    const eld::math::Vec4& b
) {
    return
        a.x == b.x &&
        a.y == b.y &&
        a.z == b.z &&
        a.w == b.w;
}


bool sameMaterial(
    const eld::render::RenderMaterial& a,
    const eld::render::RenderMaterial& b
) {
    return
        sameColor(
            a.baseColor,
            b.baseColor
        ) &&

        a.texture ==
            b.texture &&

        a.sampler.filter ==
            b.sampler.filter &&

        a.sampler.addressU ==
            b.sampler.addressU &&

        a.sampler.addressV ==
            b.sampler.addressV &&

        a.alphaMode ==
            b.alphaMode &&

        a.doubleSided ==
            b.doubleSided &&

        a.unlit ==
            b.unlit;
}


bool containsBlendedMaterial(
    const eld::render::ModelResource& model
) {
    for (
        const auto& material :
        model.materials
    ) {
        if (
            material.alphaMode ==
            eld::render::AlphaMode::Blended
        ) {
            return true;
        }
    }

    return false;
}


int chunkCoordinate(
    float position
) {
    const int value =
        static_cast<int>(
            std::floor(
                position /
                ChunkSize
            )
        );

    return std::clamp(
        value,
        0,
        ChunksPerAxis - 1
    );
}


std::size_t chunkIndex(
    const eld::render::Transform& transform
) {
    const int x =
        chunkCoordinate(
            transform.position.x
        );

    // Render Z points south, while map Y points north.
    const int y =
        chunkCoordinate(
            -transform.position.z
        );

    return static_cast<std::size_t>(
        y * ChunksPerAxis +
        x
    );
}


eld::math::Vec3 transformNormal(
    const eld::math::Mat4& matrix,
    const eld::math::Vec3& normal
) {
    const auto transformed =
        matrix.transform({
            normal.x,
            normal.y,
            normal.z,
            0.0f
        });

    return eld::math::Vec3{
        transformed.x,
        transformed.y,
        transformed.z
    }.normalized();
}


BatchBucket& findOrCreateBucket(
    ChunkBatch& chunk,

    const eld::render::RenderMaterial&
        material,

    float depthBias
) {
    for (
        auto& bucket :
        chunk.buckets
    ) {
        if (
            bucket.depthBias ==
                depthBias &&

            sameMaterial(
                bucket.material,
                material
            )
        ) {
            return bucket;
        }
    }

    chunk.buckets.push_back({
        material,
        depthBias,
        {},
        {}
    });

    return chunk.buckets.back();
}


void appendSectionGeometry(
    BatchBucket& bucket,

    const eld::render::RenderMesh& mesh,

    std::uint32_t firstIndex,
    std::uint32_t indexCount,

    const eld::math::Mat4& transform
) {
    if (
        firstIndex >
        mesh.indices.size()
    ) {
        return;
    }

    const std::size_t remaining =
        mesh.indices.size() -
        firstIndex;

    if (
        indexCount >
        remaining
    ) {
        return;
    }


    for (
        std::uint32_t i = 0;
        i < indexCount;
        ++i
    ) {
        const std::uint32_t
            sourceIndex =
                mesh.indices.at(
                    firstIndex + i
                );

        if (
            sourceIndex >=
            mesh.vertices.size()
        ) {
            continue;
        }

        auto vertex =
            mesh.vertices.at(
                sourceIndex
            );

        vertex.position =
            transform.transformPoint(
                vertex.position
            );

        vertex.normal =
            transformNormal(
                transform,
                vertex.normal
            );

        const std::uint32_t
            destinationIndex =
                static_cast<std::uint32_t>(
                    bucket.vertices.size()
                );

        bucket.vertices.push_back(
            vertex
        );

        bucket.indices.push_back(
            destinationIndex
        );
    }
}


void appendObject(
    ChunkBatch& chunk,

    const eld::render::RenderObject& object,

    const eld::render::ModelResource& model
) {
    const auto transform =
        eld::render::buildModelMatrix(
            object.transform
        );


    for (
        const auto& mesh :
        model.meshes
    ) {
        if (
            mesh.sections.empty()
        ) {
            static const
                eld::render::RenderMaterial
                    DefaultMaterial{};

            auto& bucket =
                findOrCreateBucket(
                    chunk,
                    DefaultMaterial,
                    0.0f
                );

            appendSectionGeometry(
                bucket,
                mesh,
                0,
                static_cast<std::uint32_t>(
                    mesh.indices.size()
                ),
                transform
            );

            continue;
        }


        for (
            const auto& section :
            mesh.sections
        ) {
            eld::render::RenderMaterial
                material{};

            if (
                section.materialIndex <
                model.materials.size()
            ) {
                material =
                    model.materials.at(
                        section.materialIndex
                    );
            }

            auto& bucket =
                findOrCreateBucket(
                    chunk,
                    material,
                    section.depthBias
                );

            appendSectionGeometry(
                bucket,
                mesh,
                section.firstIndex,
                section.indexCount,
                transform
            );
        }
    }
}


eld::render::ModelResource
buildChunkModel(
    ChunkBatch& chunk,

    std::size_t& sectionCount
) {
    eld::render::ModelResource
        model;

    eld::render::RenderMesh
        mesh;


    for (
        auto& bucket :
        chunk.buckets
    ) {
        if (
            bucket.indices.empty()
        ) {
            continue;
        }


        const auto materialIndex =
            static_cast<std::uint32_t>(
                model.materials.size()
            );

        model.materials.push_back(
            bucket.material
        );


        const auto firstIndex =
            static_cast<std::uint32_t>(
                mesh.indices.size()
            );

        const auto vertexOffset =
            static_cast<std::uint32_t>(
                mesh.vertices.size()
            );


        mesh.vertices.insert(
            mesh.vertices.end(),
            bucket.vertices.begin(),
            bucket.vertices.end()
        );


        for (
            const auto index :
            bucket.indices
        ) {
            mesh.indices.push_back(
                vertexOffset +
                index
            );
        }


        mesh.sections.push_back({
            firstIndex,
            static_cast<std::uint32_t>(
                bucket.indices.size()
            ),
            materialIndex,
            bucket.depthBias
        });

        ++sectionCount;
    }


    if (
        !mesh.indices.empty()
    ) {
        model.meshes.push_back(
            std::move(mesh)
        );
    }


    return model;
}

}


LocationBatchBuildResult
LocationBatchBuilder::build(
    const std::vector<
        eld::render::RenderObject
    >& objects,

    const eld::render::ModelManager&
        models
) const {
    LocationBatchBuildResult result;

    result.sourceObjects =
        objects.size();


    std::vector<ChunkBatch>
        chunks(
            ChunksPerAxis *
            ChunksPerAxis
        );


    for (
        const auto& object :
        objects
    ) {
        if (
            !object.visible ||
            !models.isValid(
                object.model
            )
        ) {
            result.passthroughObjects
                .push_back(
                    object
                );

            continue;
        }


        const auto& model =
            models.get(
                object.model
            );


        // Keep true alpha-blended models separate for now.
        // Their draw order can affect the final image.
        if (
            containsBlendedMaterial(
                model
            )
        ) {
            result.passthroughObjects
                .push_back(
                    object
                );

            continue;
        }


        auto& chunk =
            chunks.at(
                chunkIndex(
                    object.transform
                )
            );

        appendObject(
            chunk,
            object,
            model
        );

        ++result.batchedObjects;
    }


    for (
        auto& chunk :
        chunks
    ) {
        auto model =
            buildChunkModel(
                chunk,
                result.batchSections
            );

        if (
            model.meshes.empty()
        ) {
            continue;
        }

        result.batches.push_back(
            std::move(model)
        );
    }


    return result;
}

}
