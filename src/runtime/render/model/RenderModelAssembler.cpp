#include "RenderModelAssembler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eld::render {

namespace {

constexpr float Epsilon = 0.000001f;

eld::math::Vec3 toGraphicsPosition(
    const eld::model::Vertex& vertex
) {
    return {
        vertex.x,
        -vertex.y,
        vertex.z
    };
}

eld::math::Vec3 normalizedOrDefault(
    const eld::math::Vec3& value
) {
    const float length = value.length();

    if (length <= Epsilon) {
        return {0.0f, 0.0f, 1.0f};
    }

    return value * (1.0f / length);
}

float hueToRgb(
    float p,
    float q,
    float t
) {
    if (t < 0.0f) {
        t += 1.0f;
    }

    if (t > 1.0f) {
        t -= 1.0f;
    }

    if (t < 1.0f / 6.0f) {
        return p + (q - p) * 6.0f * t;
    }

    if (t < 1.0f / 2.0f) {
        return q;
    }

    if (t < 2.0f / 3.0f) {
        return p + (q - p) *
            (2.0f / 3.0f - t) * 6.0f;
    }

    return p;
}

eld::math::Vec4 convertColor(
    std::uint16_t color,
    std::uint8_t alpha
) {
    const float h =
        static_cast<float>((color >> 10) & 0x3F) /
        64.0f;

    const float s =
        static_cast<float>((color >> 7) & 0x07) /
        7.0f;

    const float l =
        static_cast<float>(color & 0x7F) /
        127.0f;

    float red = l;
    float green = l;
    float blue = l;

    if (s != 0.0f) {
        const float q =
            l < 0.5f
                ? l * (1.0f + s)
                : l + s - l * s;

        const float p = 2.0f * l - q;

        red = hueToRgb(p, q, h + 1.0f / 3.0f);
        green = hueToRgb(p, q, h);
        blue = hueToRgb(p, q, h - 1.0f / 3.0f);
    }

    return {
        std::clamp(red, 0.0f, 1.0f),
        std::clamp(green, 0.0f, 1.0f),
        std::clamp(blue, 0.0f, 1.0f),
        1.0f - static_cast<float>(alpha) / 255.0f
    };
}

void validateFace(
    const eld::model::Face& face,
    const eld::model::Model& source
) {
    if (
        face.a >= source.vertices.size() ||
        face.b >= source.vertices.size() ||
        face.c >= source.vertices.size()
    ) {
        throw std::invalid_argument(
            "Model face vertex index is invalid"
        );
    }
}

eld::math::Vec3 calculateFaceNormal(
    const eld::model::Face& face,
    const eld::model::Model& source
) {
    const eld::math::Vec3 a =
        toGraphicsPosition(source.vertices.at(face.a));

    const eld::math::Vec3 b =
        toGraphicsPosition(source.vertices.at(face.b));

    const eld::math::Vec3 c =
        toGraphicsPosition(source.vertices.at(face.c));

    return normalizedOrDefault(
        (b - a).cross(c - a)
    );
}

eld::math::Vec2 projectUv(
    const eld::math::Vec3& point,
    const eld::math::Vec3& origin,
    const eld::math::Vec3& uPoint,
    const eld::math::Vec3& vPoint
) {
    const eld::math::Vec3 uAxis = uPoint - origin;
    const eld::math::Vec3 vAxis = vPoint - origin;
    const eld::math::Vec3 relative = point - origin;

    const float uu = uAxis.dot(uAxis);
    const float uv = uAxis.dot(vAxis);
    const float vv = vAxis.dot(vAxis);

    const float relativeU = relative.dot(uAxis);
    const float relativeV = relative.dot(vAxis);

    const float denominator = uu * vv - uv * uv;

    if (std::abs(denominator) <= Epsilon) {
        return {};
    }

    return {
        (vv * relativeU - uv * relativeV) /
            denominator,
        (uu * relativeV - uv * relativeU) /
            denominator
    };
}

std::array<eld::math::Vec2, 3> calculateFaceUvs(
    const eld::model::Face& face,
    const eld::model::Model& source
) {
    std::array<eld::math::Vec2, 3> result{
        eld::math::Vec2{0.0f, 0.0f},
        eld::math::Vec2{1.0f, 0.0f},
        eld::math::Vec2{0.0f, 1.0f}
    };

    if (!face.textureMappingIndex.has_value()) {
        return result;
    }

    const std::size_t mappingIndex =
        *face.textureMappingIndex;

    if (mappingIndex >= source.textureMappings.size()) {
        throw std::invalid_argument(
            "Face texture mapping index is invalid"
        );
    }

    const eld::model::TextureMapping& mapping =
        source.textureMappings.at(mappingIndex);

    if (
        mapping.originVertex >= source.vertices.size() ||
        mapping.uVertex >= source.vertices.size() ||
        mapping.vVertex >= source.vertices.size()
    ) {
        throw std::invalid_argument(
            "Texture mapping vertex index is invalid"
        );
    }

    const eld::math::Vec3 origin =
        toGraphicsPosition(
            source.vertices.at(mapping.originVertex)
        );

    const eld::math::Vec3 uPoint =
        toGraphicsPosition(
            source.vertices.at(mapping.uVertex)
        );

    const eld::math::Vec3 vPoint =
        toGraphicsPosition(
            source.vertices.at(mapping.vVertex)
        );

    const std::array<std::uint32_t, 3> indices{
        face.a,
        face.b,
        face.c
    };

    for (std::size_t corner = 0; corner < 3; corner++) {
        result.at(corner) =
            projectUv(
                toGraphicsPosition(
                    source.vertices.at(indices.at(corner))
                ),
                origin,
                uPoint,
                vPoint
            );
    }

    return result;
}

std::uint32_t findOrCreateMaterial(
    RenderModel& model,
    const std::optional<TextureHandle>& texture,
    AlphaMode alphaMode
) {
    for (
        std::size_t i = 0;
        i < model.materials.size();
        i++
    ) {
        const RenderMaterial& material =
            model.materials.at(i);

        if (
            material.texture == texture &&
            material.alphaMode == alphaMode
        ) {
            return static_cast<std::uint32_t>(i);
        }
    }

    RenderMaterial material{};
    material.texture = texture;
    material.alphaMode = alphaMode;

    material.sampler.filter =
        TextureFilter::Nearest;

    material.sampler.addressU =
        TextureAddressMode::Clamp;

    material.sampler.addressV =
        TextureAddressMode::Repeat;

    model.materials.push_back(material);

    return static_cast<std::uint32_t>(
        model.materials.size() - 1
    );
}

constexpr float PriorityDepthBiasPerLevel = 0.001f;

float depthBiasForPriority(
    std::uint8_t priority
) {
    return
        -static_cast<float>(priority) *
        PriorityDepthBiasPerLevel;
}

void appendSection(
    RenderMesh& mesh,
    std::uint32_t firstIndex,
    std::uint32_t materialIndex,
    float depthBias
) {
    if (!mesh.sections.empty()) {
        RenderMeshSection& previous =
            mesh.sections.back();

        if (
            previous.materialIndex == materialIndex &&
            previous.depthBias == depthBias &&
            previous.firstIndex + previous.indexCount ==
                firstIndex
        ) {
            previous.indexCount += 3;
            return;
        }
    }

    mesh.sections.push_back({
        firstIndex,
        3,
        materialIndex,
        depthBias
    });
}

}

RenderModelAssembler::RenderModelAssembler(
    TextureResolver& textureResolver
)
    : textureResolver_(textureResolver) {
}

RenderModel RenderModelAssembler::assemble(
    const eld::model::Model& source
) {
    RenderModel model;
    RenderMesh mesh;

    std::vector<eld::math::Vec3> faceNormals(
        source.faces.size()
    );

    std::vector<eld::math::Vec3> vertexNormals(
        source.vertices.size()
    );

    for (
        std::size_t faceIndex = 0;
        faceIndex < source.faces.size();
        faceIndex++
    ) {
        const eld::model::Face& face =
            source.faces.at(faceIndex);

        validateFace(face, source);

        const eld::math::Vec3 faceNormal =
            calculateFaceNormal(face, source);

        faceNormals.at(faceIndex) = faceNormal;

        const bool flatShaded =
            (face.renderType & 1) != 0;

        if (!flatShaded) {
            vertexNormals.at(face.a) =
                vertexNormals.at(face.a) + faceNormal;

            vertexNormals.at(face.b) =
                vertexNormals.at(face.b) + faceNormal;

            vertexNormals.at(face.c) =
                vertexNormals.at(face.c) + faceNormal;
        }
    }

    for (eld::math::Vec3& normal : vertexNormals) {
        normal = normalizedOrDefault(normal);
    }

    mesh.vertices.reserve(source.faces.size() * 3);
    mesh.indices.reserve(source.faces.size() * 3);

    for (
        std::size_t faceIndex = 0;
        faceIndex < source.faces.size();
        faceIndex++
    ) {
        const eld::model::Face& face =
            source.faces.at(faceIndex);

        std::optional<TextureHandle> texture;

        if (face.textureId.has_value()) {
            texture =
                textureResolver_.resolve(
                    *face.textureId
                );
        }

        AlphaMode alphaMode = AlphaMode::Opaque;

        if (face.alpha != 0) {
            alphaMode = AlphaMode::Blended;
        }
        else if (texture.has_value()) {
            alphaMode = AlphaMode::Masked;
        }

        const std::uint32_t materialIndex =
            findOrCreateMaterial(
                model,
                texture,
                alphaMode
            );

        eld::math::Vec4 color{
            1.0f,
            1.0f,
            1.0f,
            1.0f -
                static_cast<float>(face.alpha) /
                255.0f
        };

        if (!texture.has_value()) {
            color =
                convertColor(
                    face.color,
                    face.alpha
                );
        }

        const std::array<eld::math::Vec2, 3> uvs =
            calculateFaceUvs(face, source);

        const bool flatShaded =
            (face.renderType & 1) != 0;

        const std::array<std::uint32_t, 3> sourceIndices{
            face.a,
            face.b,
            face.c
        };

        const std::uint32_t firstVertex =
            static_cast<std::uint32_t>(
                mesh.vertices.size()
            );

        const std::uint32_t firstIndex =
            static_cast<std::uint32_t>(
                mesh.indices.size()
            );

        for (std::size_t corner = 0; corner < 3; corner++) {
            const std::uint32_t sourceIndex =
                sourceIndices.at(corner);

            mesh.vertices.push_back({
                toGraphicsPosition(
                    source.vertices.at(sourceIndex)
                ),
                flatShaded
                    ? faceNormals.at(faceIndex)
                    : vertexNormals.at(sourceIndex),
                uvs.at(corner),
                color
            });

            mesh.indices.push_back(
                firstVertex +
                static_cast<std::uint32_t>(corner)
            );
        }

        appendSection(
            mesh,
            firstIndex,
            materialIndex,
            depthBiasForPriority(
                face.priority
            )
        );
    }

    model.meshes.push_back(std::move(mesh));

    return model;
}

}
