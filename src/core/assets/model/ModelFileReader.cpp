#include "ModelFileReader.h"

#include "../../io/ByteBuffer.h"

namespace rf::model {

namespace {

constexpr int ModelFooterSize = 18;

constexpr uint32_t OptionalChunkPresent = 1;
constexpr uint32_t PerFacePriority = 255;

constexpr int FaceColorSize = 2;
constexpr int TextureTriangleSize = 6;

}

std::optional<ModelFile> ModelFileReader::read(
    const std::vector<uint8_t>& payload
) const {
    if (!validatePayload(payload)) {
        return std::nullopt;
    }

    ModelFooter footer =
        readFooter(payload);

    ModelLayout layout =
        calculateLayout(footer);

    return ModelFile {
        payload,
        footer,
        layout
    };
}

bool ModelFileReader::validatePayload(
    const std::vector<uint8_t>& payload
) const {
    return payload.size() >= ModelFooterSize;
}

ModelFooter ModelFileReader::readFooter(
    const std::vector<uint8_t>& payload
) const {
    rf::io::ByteBuffer buffer(payload);

    buffer.setPosition(
        static_cast<int>(payload.size()) -
        ModelFooterSize
    );

    ModelFooter footer {};

    footer.vertexCount = buffer.readU16();
    footer.triangleCount = buffer.readU16();

    footer.textureTriangleCount = buffer.readU8();

    footer.textureFlag = buffer.readU8();
    footer.priorityFlag = buffer.readU8();
    footer.alphaFlag = buffer.readU8();

    footer.triangleSkinFlag = buffer.readU8();
    footer.vertexSkinFlag = buffer.readU8();

    footer.xDataLength = buffer.readU16();
    footer.yDataLength = buffer.readU16();
    footer.zDataLength = buffer.readU16();

    footer.triangleDataLength = buffer.readU16();

    return footer;
}

ModelLayout ModelFileReader::calculateLayout(
    const ModelFooter& footer
) const {
    ModelLayout layout {};

    int offset = 0;

    layout.vertexFlagsOffset = offset;
    offset += footer.vertexCount;

    layout.triangleTypesOffset = offset;
    offset += footer.triangleCount;

    layout.trianglePrioritiesOffset = offset;

    if (footer.priorityFlag == PerFacePriority) {
        offset += footer.triangleCount;
    }

    layout.triangleSkinsOffset = offset;

    if (footer.triangleSkinFlag == OptionalChunkPresent) {
        offset += footer.triangleCount;
    }

    layout.texturePointersOffset = offset;

    if (footer.textureFlag == OptionalChunkPresent) {
        offset += footer.triangleCount;
    }

    layout.vertexSkinsOffset = offset;

    if (footer.vertexSkinFlag == OptionalChunkPresent) {
        offset += footer.vertexCount;
    }

    layout.triangleAlphasOffset = offset;

    if (footer.alphaFlag == OptionalChunkPresent) {
        offset += footer.triangleCount;
    }

    layout.triangleDataOffset = offset;
    offset += footer.triangleDataLength;

    layout.triangleColorsOffset = offset;
    offset += footer.triangleCount * FaceColorSize;

    layout.textureDataOffset = offset;
    offset +=
        footer.textureTriangleCount *
        TextureTriangleSize;

    layout.xDataOffset = offset;
    offset += footer.xDataLength;

    layout.yDataOffset = offset;
    offset += footer.yDataLength;

    layout.zDataOffset = offset;
    offset += footer.zDataLength;

    return layout;
}

}
