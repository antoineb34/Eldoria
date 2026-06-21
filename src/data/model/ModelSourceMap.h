#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace eld::model {

struct VertexSource {
    std::size_t flagUnitIndex = 0;

    std::optional<std::size_t> xDeltaUnitIndex;
    std::optional<std::size_t> yDeltaUnitIndex;
    std::optional<std::size_t> zDeltaUnitIndex;
    std::optional<std::size_t> skinUnitIndex;
};

struct FaceSource {
    std::size_t triangleTypeUnitIndex = 0;
    std::vector<std::size_t> triangleDeltaUnitIndices;

    std::size_t colorUnitIndex = 0;

    std::optional<std::size_t> priorityUnitIndex;
    std::optional<std::size_t> alphaUnitIndex;
    std::optional<std::size_t> skinUnitIndex;
    std::optional<std::size_t> textureInfoUnitIndex;
};

struct TextureMappingSource {
    std::size_t textureDataUnitIndex = 0;
};

struct ModelSourceMap {
    std::vector<VertexSource> vertices;
    std::vector<FaceSource> faces;
    std::vector<TextureMappingSource> textureMappings;
};

}
