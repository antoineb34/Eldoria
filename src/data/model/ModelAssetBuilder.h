#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ModelAsset.h"
#include "ModelFile.h"
#include "ModelSourceMap.h"

namespace eld::model {

class ModelAssetBuilder {
public:
    ModelAsset build(
        const ModelFile& file
    ) const;

    ModelAsset build(
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

    std::vector<Vertex> buildVertices(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    std::vector<Face> buildFaces(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    std::vector<TextureMapping> buildTextureMappings(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;

    ModelAsset buildAsset(
        const ModelFile& file,
        ModelSourceMap* sourceMap
    ) const;
};

}
