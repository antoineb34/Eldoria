#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "ModelFile.h"

namespace eld::model {

class ModelFileParser {
public:
    std::optional<ModelFile> parse(
        const std::vector<std::uint8_t>& payload
    ) const;

private:
    bool validatePayload(
        const std::vector<std::uint8_t>& payload
    ) const;

    ModelFooter readFooter(
        const std::vector<std::uint8_t>& payload
    ) const;

    ModelLayout calculateLayout(
        const ModelFooter& footer
    ) const;

    bool validateLayout(
        const std::vector<std::uint8_t>& payload,
        const ModelFooter& footer,
        const ModelLayout& layout
    ) const;

    ModelSections parseSections(
        const ModelFile& file
    ) const;

    ModelSection<VertexFlagUnit> parseVertexFlags(
        const ModelFile& file
    ) const;

    ModelSection<VertexSkinUnit> parseVertexSkins(
        const ModelFile& file
    ) const;

    ModelSection<VertexDeltaUnit> parseVertexDeltas(
        const ModelFile& file,
        int offset,
        std::uint32_t dataLength
    ) const;

    ModelSection<TriangleTypeUnit> parseTriangleTypes(
        const ModelFile& file
    ) const;

    ModelSection<TrianglePriorityUnit> parseTrianglePriorities(
        const ModelFile& file
    ) const;

    ModelSection<TriangleSkinUnit> parseTriangleSkins(
        const ModelFile& file
    ) const;

    ModelSection<TexturePointerUnit> parseTexturePointers(
        const ModelFile& file
    ) const;

    ModelSection<TriangleAlphaUnit> parseTriangleAlphas(
        const ModelFile& file
    ) const;

    ModelSection<TriangleColorUnit> parseTriangleColors(
        const ModelFile& file
    ) const;

    ModelSection<TriangleIndexDeltaUnit> parseTriangleData(
        const ModelFile& file
    ) const;

    ModelSection<TextureTriangleUnit> parseTextureData(
        const ModelFile& file
    ) const;
};

}
