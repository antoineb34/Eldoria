#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ModelFile.h"
#include "ModelMesh.h"
#include "ModelSourceMap.h"

namespace eld::model {

class ModelDecoder {
public:
    ModelMesh decode(
        const ModelFile& file
    ) const;

    ModelMesh decode(
        const ModelFile& file,
        ModelSourceMap& sourceMap
    ) const;

private:
    struct TriangleState {
        std::uint32_t a = 0;
        std::uint32_t b = 0;
        std::uint32_t c = 0;

        int lastIndex = 0;
        std::size_t deltaUnitIndex = 0;
    };

    int readTriangleIndex(
        const ModelSections& sections,
        TriangleState& state,
        FaceSource* faceSource
    ) const;

    void applyTriangleType(
        std::uint8_t triangleType,
        const ModelSections& sections,
        TriangleState& state,
        FaceSource* faceSource
    ) const;

    std::vector<Vertex> decodeVertices(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    std::vector<Face> decodeFaces(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    std::vector<TextureMapping> decodeTextureMappings(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    ModelMesh decodeMesh(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;
};

}
