#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::model {

struct ModelFooter {
    std::uint32_t vertexCount = 0;
    std::uint32_t triangleCount = 0;
    std::uint32_t textureTriangleCount = 0;

    std::uint32_t textureFlag = 0;
    std::uint32_t priorityFlag = 0;
    std::uint32_t alphaFlag = 0;
    std::uint32_t triangleSkinFlag = 0;
    std::uint32_t vertexSkinFlag = 0;

    std::uint32_t xDataLength = 0;
    std::uint32_t yDataLength = 0;
    std::uint32_t zDataLength = 0;
    std::uint32_t triangleDataLength = 0;
};

struct ModelLayout {
    int vertexFlagsOffset = 0;
    int triangleTypesOffset = 0;
    int trianglePrioritiesOffset = 0;
    int triangleSkinsOffset = 0;
    int texturePointersOffset = 0;
    int vertexSkinsOffset = 0;
    int triangleAlphasOffset = 0;
    int triangleDataOffset = 0;
    int triangleColorsOffset = 0;
    int textureDataOffset = 0;
    int xDataOffset = 0;
    int yDataOffset = 0;
    int zDataOffset = 0;
};

struct ModelUnit {
    std::size_t index = 0;
    std::size_t offset = 0;
    std::size_t size = 0;

    std::size_t end() const {
        return offset + size;
    }

    bool present() const {
        return size > 0;
    }
};

template <typename TUnit>
struct ModelSection {
    std::size_t offset = 0;
    std::size_t size = 0;

    std::vector<TUnit> units;

    std::size_t end() const {
        return offset + size;
    }

    bool present() const {
        return size > 0;
    }

    std::size_t unitCount() const {
        return units.size();
    }
};

struct VertexFlagUnit {
    ModelUnit source;
    std::uint8_t flags = 0;

    bool hasXDelta() const {
        return (flags & 0x1) != 0;
    }

    bool hasYDelta() const {
        return (flags & 0x2) != 0;
    }

    bool hasZDelta() const {
        return (flags & 0x4) != 0;
    }
};

struct VertexSkinUnit {
    ModelUnit source;
    std::uint8_t skin = 0;
};

struct VertexDeltaUnit {
    ModelUnit source;
    int delta = 0;
};

struct TriangleTypeUnit {
    ModelUnit source;
    std::uint8_t type = 0;
};

struct TrianglePriorityUnit {
    ModelUnit source;
    std::uint8_t priority = 0;
};

struct TriangleSkinUnit {
    ModelUnit source;
    std::uint8_t skin = 0;
};

struct TexturePointerUnit {
    ModelUnit source;
    std::uint8_t value = 0;
};

struct TriangleAlphaUnit {
    ModelUnit source;
    std::uint8_t alpha = 0;
};

struct TriangleColorUnit {
    ModelUnit source;
    std::uint16_t color = 0;
};

struct TriangleIndexDeltaUnit {
    ModelUnit source;
    int delta = 0;
};

struct TextureTriangleUnit {
    ModelUnit source;

    std::uint16_t originVertex = 0;
    std::uint16_t uVertex = 0;
    std::uint16_t vVertex = 0;
};

struct ModelSections {
    ModelSection<VertexFlagUnit> vertexFlags;
    ModelSection<VertexSkinUnit> vertexSkins;

    ModelSection<VertexDeltaUnit> xData;
    ModelSection<VertexDeltaUnit> yData;
    ModelSection<VertexDeltaUnit> zData;

    ModelSection<TriangleTypeUnit> triangleTypes;
    ModelSection<TrianglePriorityUnit> trianglePriorities;
    ModelSection<TriangleSkinUnit> triangleSkins;
    ModelSection<TexturePointerUnit> texturePointers;
    ModelSection<TriangleAlphaUnit> triangleAlphas;
    ModelSection<TriangleColorUnit> triangleColors;
    ModelSection<TriangleIndexDeltaUnit> triangleData;

    ModelSection<TextureTriangleUnit> textureData;
};

struct ModelFile {
    std::vector<std::uint8_t> payload;

    ModelFooter footer;
    ModelLayout layout;
    ModelSections sections;
};

}
