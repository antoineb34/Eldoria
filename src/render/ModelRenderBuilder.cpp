#include "ModelRenderBuilder.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "color/Color.h"

namespace eld::render {

namespace {

struct MaterialKey {
    int textureId = -1;
    BlendMode blendMode =
        BlendMode::Opaque;

    bool operator==(
        const MaterialKey& other
    ) const {
        return
            textureId ==
                other.textureId &&
            blendMode ==
                other.blendMode;
    }
};

struct MaterialKeyHash {
    std::size_t operator()(
        const MaterialKey& key
    ) const {
        return
            static_cast<std::size_t>(
                key.textureId + 1
            ) *
                31U +
            static_cast<std::size_t>(
                key.blendMode
            );
    }
};

struct MaterialBucket {
    Material material;

    std::vector<
        const eld::model::Face*
    > faces;
};

Vec3 toRenderPosition(
    const eld::model::Vertex& vertex
) {
    return Vec3{
        vertex.x,
        -vertex.y,
        vertex.z
    };
}

bool validVertexIndex(
    std::uint32_t index,
    const eld::model::ModelMesh& model
) {
    return
        index <
        model.vertices.size();
}

bool validFace(
    const eld::model::Face& face,
    const eld::model::ModelMesh& model
) {
    return
        validVertexIndex(
            face.a,
            model
        ) &&
        validVertexIndex(
            face.b,
            model
        ) &&
        validVertexIndex(
            face.c,
            model
        );
}

const eld::texture::TextureAsset*
findTexture(
    std::uint16_t id,
    const std::unordered_map<
        std::uint16_t,
        const eld::texture::TextureAsset*
    >& textures
) {
    const auto entry =
        textures.find(id);

    if (
        entry == textures.end()
    ) {
        return nullptr;
    }

    return entry->second;
}

bool hasTransparency(
    const eld::texture::TextureAsset& texture
) {
    for (
        const eld::texture::RgbaPixel& pixel :
        texture.pixels
    ) {
        if (pixel.alpha < 255) {
            return true;
        }
    }

    return false;
}

Material createMaterial(
    const MaterialKey& key,
    const eld::texture::TextureAsset* texture
) {
    Material material;

    material.blendMode =
        key.blendMode;

    if (key.textureId < 0) {
        return material;
    }

    material.albedoTexture =
        texture;

    material.sampler.addressU =
        TextureAddressMode::Clamp;

    material.sampler.addressV =
        TextureAddressMode::Repeat;

    material.sampler.filter =
        TextureFilter::Nearest;

    if (texture == nullptr) {
        material.baseColor =
            Vec4{
                1.0f,
                0.0f,
                1.0f,
                1.0f
            };
    }

    return material;
}

Vec2 projectUv(
    const Vec3& point,
    const Vec3& origin,
    const Vec3& uVertex,
    const Vec3& vVertex
) {
    const Vec3 basisU =
        uVertex -
        origin;

    const Vec3 basisV =
        vVertex -
        origin;

    const Vec3 relative =
        point -
        origin;

    const float uu =
        basisU.dot(
            basisU
        );

    const float uv =
        basisU.dot(
            basisV
        );

    const float vv =
        basisV.dot(
            basisV
        );

    const float projectedU =
        relative.dot(
            basisU
        );

    const float projectedV =
        relative.dot(
            basisV
        );

    const float determinant =
        uu * vv -
        uv * uv;

    if (
        std::abs(determinant) <
        0.0001f
    ) {
        return {};
    }

    return Vec2{
        (
            projectedU * vv -
            projectedV * uv
        ) /
            determinant,
        (
            projectedV * uu -
            projectedU * uv
        ) /
            determinant
    };
}

std::array<Vec2, 3> buildUvs(
    const eld::model::ModelMesh& model,
    const eld::model::Face& face,
    const std::array<Vec3, 3>& positions
) {
    const std::array<Vec2, 3>
        fallback{
            Vec2{0.0f, 0.0f},
            Vec2{1.0f, 0.0f},
            Vec2{0.0f, 1.0f}
        };

    if (
        !face.textureMappingIndex.has_value()
    ) {
        return fallback;
    }

    const std::size_t mappingIndex =
        *face.textureMappingIndex;

    if (
        mappingIndex >=
        model.textureMappings.size()
    ) {
        return fallback;
    }

    const eld::model::TextureMapping&
        mapping =
            model.textureMappings[
                mappingIndex
            ];

    if (
        !validVertexIndex(
            mapping.originVertex,
            model
        ) ||
        !validVertexIndex(
            mapping.uVertex,
            model
        ) ||
        !validVertexIndex(
            mapping.vVertex,
            model
        )
    ) {
        return fallback;
    }

    const Vec3 origin =
        toRenderPosition(
            model.vertices[
                mapping.originVertex
            ]
        );

    const Vec3 uVertex =
        toRenderPosition(
            model.vertices[
                mapping.uVertex
            ]
        );

    const Vec3 vVertex =
        toRenderPosition(
            model.vertices[
                mapping.vVertex
            ]
        );

    const Vec3 basisU =
        uVertex -
        origin;

    const Vec3 basisV =
        vVertex -
        origin;

    const float determinant =
        basisU.dot(basisU) *
            basisV.dot(basisV) -
        basisU.dot(basisV) *
            basisU.dot(basisV);

    if (
        std::abs(determinant) <
        0.0001f
    ) {
        return fallback;
    }

    return std::array<Vec2, 3>{
        projectUv(
            positions[0],
            origin,
            uVertex,
            vVertex
        ),
        projectUv(
            positions[1],
            origin,
            uVertex,
            vVertex
        ),
        projectUv(
            positions[2],
            origin,
            uVertex,
            vVertex
        )
    };
}

Vec4 buildVertexColor(
    const eld::model::Face& face
) {
    const float alpha =
        static_cast<float>(
            255 -
            face.alpha
        ) /
        255.0f;

    if (face.textureId.has_value()) {
        return Vec4{
            1.0f,
            1.0f,
            1.0f,
            alpha
        };
    }

    const RgbColor color =
        rsColorToRgb(
            face.color
        );

    return Vec4{
        static_cast<float>(
            color.r
        ) /
            255.0f,
        static_cast<float>(
            color.g
        ) /
            255.0f,
        static_cast<float>(
            color.b
        ) /
            255.0f,
        alpha
    };
}

}

RenderModel ModelRenderBuilder::build(
    const eld::model::ModelMesh& model,
    const std::unordered_map<
        std::uint16_t,
        const eld::texture::TextureAsset*
    >& textures
) const {
    std::vector<MaterialBucket> buckets;

    std::unordered_map<
        MaterialKey,
        std::size_t,
        MaterialKeyHash
    > bucketIndices;

    for (
        const eld::model::Face& face :
        model.faces
    ) {
        if (!validFace(face, model)) {
            continue;
        }

        const int textureId =
            face.textureId.has_value()
                ? static_cast<int>(
                    *face.textureId
                )
                : -1;

        const eld::texture::TextureAsset*
            texture = nullptr;

        if (textureId >= 0) {
            texture =
                findTexture(
                    static_cast<std::uint16_t>(
                        textureId
                    ),
                    textures
                );
        }

        const bool transparent =
            face.alpha > 0 ||
            (
                texture != nullptr &&
                hasTransparency(
                    *texture
                )
            );

        const MaterialKey key{
            .textureId = textureId,
            .blendMode =
                transparent
                    ? BlendMode::Alpha
                    : BlendMode::Opaque
        };

        auto entry =
            bucketIndices.find(
                key
            );

        if (
            entry ==
            bucketIndices.end()
        ) {
            const std::size_t index =
                buckets.size();

            bucketIndices.emplace(
                key,
                index
            );

            buckets.push_back(
                MaterialBucket{
                    .material =
                        createMaterial(
                            key,
                            texture
                        ),
                    .faces = {}
                }
            );

            entry =
                bucketIndices.find(
                    key
                );
        }

        buckets[
            entry->second
        ].faces.push_back(
            &face
        );
    }

    RenderModel renderModel;

    renderModel.materials.reserve(
        buckets.size()
    );

    for (
        const MaterialBucket& bucket :
        buckets
    ) {
        renderModel.materials.push_back(
            bucket.material
        );
    }

    for (
        std::size_t materialIndex = 0;
        materialIndex < buckets.size();
        materialIndex++
    ) {
        const MaterialBucket& bucket =
            buckets[
                materialIndex
            ];

        const std::size_t firstIndex =
            renderModel.mesh.indices.size();

        for (
            const eld::model::Face* face :
            bucket.faces
        ) {
            const std::array<Vec3, 3>
                positions{
                    toRenderPosition(
                        model.vertices[
                            face->a
                        ]
                    ),
                    toRenderPosition(
                        model.vertices[
                            face->b
                        ]
                    ),
                    toRenderPosition(
                        model.vertices[
                            face->c
                        ]
                    )
                };

            const Vec3 normal =
                (
                    positions[1] -
                    positions[0]
                )
                    .cross(
                        positions[2] -
                        positions[0]
                    )
                    .normalized();

            const std::array<Vec2, 3> uvs =
                face->textureId.has_value()
                    ? buildUvs(
                        model,
                        *face,
                        positions
                    )
                    : std::array<Vec2, 3>{};

            const Vec4 color =
                buildVertexColor(
                    *face
                );

            const std::uint32_t
                firstVertex =
                    static_cast<std::uint32_t>(
                        renderModel.mesh.vertices.size()
                    );

            for (
                std::size_t corner = 0;
                corner < 3;
                corner++
            ) {
                renderModel.mesh.vertices.push_back(
                    RenderVertex{
                        .position =
                            positions[
                                corner
                            ],
                        .normal = normal,
                        .uv =
                            uvs[
                                corner
                            ],
                        .color = color
                    }
                );
            }

            renderModel.mesh.indices.push_back(
                firstVertex
            );

            renderModel.mesh.indices.push_back(
                firstVertex + 1
            );

            renderModel.mesh.indices.push_back(
                firstVertex + 2
            );
        }

        const std::size_t indexCount =
            renderModel.mesh.indices.size() -
            firstIndex;

        if (indexCount == 0) {
            continue;
        }

        renderModel.mesh.submeshes.push_back(
            RenderSubmesh{
                .firstIndex = firstIndex,
                .indexCount = indexCount,
                .materialIndex =
                    materialIndex
            }
        );
    }

    return renderModel;
}

}
