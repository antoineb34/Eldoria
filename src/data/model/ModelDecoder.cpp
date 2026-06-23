#include "ModelDecoder.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace eld::model {

int ModelDecoder::readTriangleIndex(
    const ModelSections& sections,
    TriangleState& state,
    FaceSource* faceSource
) const {
    const std::size_t unitIndex =
        state.deltaUnitIndex++;

    const int index =
        state.lastIndex +
        sections.triangleData.units.at(
            unitIndex
        ).delta;

    state.lastIndex =
        index;

    if (faceSource != nullptr) {
        faceSource->triangleDeltaUnitIndices.push_back(
            unitIndex
        );
    }

    return index;
}

void ModelDecoder::applyTriangleType(
    std::uint8_t triangleType,
    const ModelSections& sections,
    TriangleState& state,
    FaceSource* faceSource
) const {
    constexpr std::uint8_t TriangleTypeNew = 1;
    constexpr std::uint8_t TriangleTypeReuseCAsB = 2;
    constexpr std::uint8_t TriangleTypeReuseCAsA = 3;
    constexpr std::uint8_t TriangleTypeSwapAB = 4;

    switch (triangleType) {
        case TriangleTypeNew:
            state.a =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            state.b =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            state.c =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            break;

        case TriangleTypeReuseCAsB:
            state.b =
                state.c;

            state.c =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            break;

        case TriangleTypeReuseCAsA:
            state.a =
                state.c;

            state.c =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            break;

        case TriangleTypeSwapAB:
            std::swap(
                state.a,
                state.b
            );

            state.c =
                static_cast<std::uint32_t>(
                    readTriangleIndex(
                        sections,
                        state,
                        faceSource
                    )
                );

            break;

        default:
            break;
    }
}

std::vector<Vertex> ModelDecoder::decodeVertices(
    const ModelFile& file,
    ModelSourceMap* sourceMap
) const {
    const ModelSections& sections =
        file.sections;

    std::vector<Vertex> vertices;

    vertices.reserve(
        file.footer.vertexCount
    );

    if (sourceMap != nullptr) {
        sourceMap->vertices.reserve(
            file.footer.vertexCount
        );
    }

    int currentX = 0;
    int currentY = 0;
    int currentZ = 0;

    std::size_t xDeltaUnitIndex = 0;
    std::size_t yDeltaUnitIndex = 0;
    std::size_t zDeltaUnitIndex = 0;

    for (
        std::size_t i = 0;
        i < file.footer.vertexCount;
        i++
    ) {
        const VertexFlagUnit& flag =
            sections.vertexFlags.units.at(
                i
            );

        VertexSource source{};

        source.flagUnitIndex =
            i;

        int deltaX = 0;
        int deltaY = 0;
        int deltaZ = 0;

        if (flag.hasXDelta()) {
            deltaX =
                sections.xData.units.at(
                    xDeltaUnitIndex
                ).delta;

            source.xDeltaUnitIndex =
                xDeltaUnitIndex;

            xDeltaUnitIndex++;
        }

        if (flag.hasYDelta()) {
            deltaY =
                sections.yData.units.at(
                    yDeltaUnitIndex
                ).delta;

            source.yDeltaUnitIndex =
                yDeltaUnitIndex;

            yDeltaUnitIndex++;
        }

        if (flag.hasZDelta()) {
            deltaZ =
                sections.zData.units.at(
                    zDeltaUnitIndex
                ).delta;

            source.zDeltaUnitIndex =
                zDeltaUnitIndex;

            zDeltaUnitIndex++;
        }

        currentX +=
            deltaX;

        currentY +=
            deltaY;

        currentZ +=
            deltaZ;

        Vertex vertex{};

        vertex.x =
            static_cast<float>(
                currentX
            );

        vertex.y =
            static_cast<float>(
                currentY
            );

        vertex.z =
            static_cast<float>(
                currentZ
            );

        if (
            i <
            sections.vertexSkins.units.size()
        ) {
            vertex.skin =
                sections.vertexSkins.units.at(
                    i
                ).skin;

            source.skinUnitIndex =
                i;
        }

        vertices.push_back(
            vertex
        );

        if (sourceMap != nullptr) {
            sourceMap->vertices.push_back(
                std::move(source)
            );
        }
    }

    return vertices;
}

std::vector<Face> ModelDecoder::decodeFaces(
    const ModelFile& file,
    ModelSourceMap* sourceMap
) const {
    constexpr std::uint8_t FlatShadingFlag = 1;
    constexpr std::uint8_t TexturedFaceFlag = 2;
    constexpr std::uint8_t TextureMappingShift = 2;

    const ModelSections& sections =
        file.sections;

    std::vector<Face> faces;

    faces.reserve(
        file.footer.triangleCount
    );

    if (sourceMap != nullptr) {
        sourceMap->faces.reserve(
            file.footer.triangleCount
        );
    }

    TriangleState triangleState{};

    for (
        std::size_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::uint8_t triangleType =
            sections.triangleTypes.units.at(
                i
            ).type;

        FaceSource source{};

        source.triangleTypeUnitIndex =
            i;

        source.colorUnitIndex =
            i;

        applyTriangleType(
            triangleType,
            sections,
            triangleState,
            sourceMap != nullptr
                ? &source
                : nullptr
        );

        Face face{};

        face.a =
            triangleState.a;

        face.b =
            triangleState.b;

        face.c =
            triangleState.c;

        face.color =
            sections.triangleColors.units.at(
                i
            ).color;

        if (
            i <
            sections.trianglePriorities.units.size()
        ) {
            face.priority =
                sections.trianglePriorities.units.at(
                    i
                ).priority;

            source.priorityUnitIndex =
                i;
        }
        else {
            face.priority =
                static_cast<std::uint8_t>(
                    file.footer.priorityFlag
                );
        }

        if (
            i <
            sections.triangleAlphas.units.size()
        ) {
            face.alpha =
                sections.triangleAlphas.units.at(
                    i
                ).alpha;

            source.alphaUnitIndex =
                i;
        }

        if (
            i <
            sections.triangleSkins.units.size()
        ) {
            face.skin =
                sections.triangleSkins.units.at(
                    i
                ).skin;

            source.skinUnitIndex =
                i;
        }

        if (
            i <
            sections.texturePointers.units.size()
        ) {
            const std::uint8_t textureInfo =
                sections.texturePointers.units.at(
                    i
                ).value;

            face.renderType =
                textureInfo &
                FlatShadingFlag;

            if (
                (
                    textureInfo &
                    TexturedFaceFlag
                ) != 0
            ) {
                face.textureId =
                    face.color;

                face.textureMappingIndex =
                    static_cast<std::uint32_t>(
                        textureInfo >>
                        TextureMappingShift
                    );
            }

            source.textureInfoUnitIndex =
                i;
        }

        faces.push_back(
            face
        );

        if (sourceMap != nullptr) {
            sourceMap->faces.push_back(
                std::move(source)
            );
        }
    }

    return faces;
}

std::vector<TextureMapping> ModelDecoder::decodeTextureMappings(
    const ModelFile& file,
    ModelSourceMap* sourceMap
) const {
    const ModelSections& sections =
        file.sections;

    std::vector<TextureMapping> mappings;

    mappings.reserve(
        sections.textureData.units.size()
    );

    if (sourceMap != nullptr) {
        sourceMap->textureMappings.reserve(
            sections.textureData.units.size()
        );
    }

    for (
        std::size_t i = 0;
        i < sections.textureData.units.size();
        i++
    ) {
        const TextureTriangleUnit& unit =
            sections.textureData.units.at(
                i
            );

        mappings.push_back(
            TextureMapping{
                static_cast<std::uint32_t>(
                    unit.originVertex
                ),
                static_cast<std::uint32_t>(
                    unit.uVertex
                ),
                static_cast<std::uint32_t>(
                    unit.vVertex
                )
            }
        );

        if (sourceMap != nullptr) {
            sourceMap->textureMappings.push_back(
                TextureMappingSource{
                    i
                }
            );
        }
    }

    return mappings;
}

ModelMesh ModelDecoder::decodeMesh(
    const ModelFile& file,
    ModelSourceMap* sourceMap
) const {
    if (sourceMap != nullptr) {
        *sourceMap =
            ModelSourceMap{};
    }

    ModelMesh mesh{};

    mesh.vertices =
        decodeVertices(
            file,
            sourceMap
        );

    mesh.faces =
        decodeFaces(
            file,
            sourceMap
        );

    mesh.textureMappings =
        decodeTextureMappings(
            file,
            sourceMap
        );

    return mesh;
}

ModelMesh ModelDecoder::decode(
    const ModelFile& file
) const {
    return decodeMesh(
        file,
        nullptr
    );
}

ModelMesh ModelDecoder::decode(
    const ModelFile& file,
    ModelSourceMap& sourceMap
) const {
    return decodeMesh(
        file,
        &sourceMap
    );
}

}
