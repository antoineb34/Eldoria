#include "FaceDecoder.h"

#include <utility>
#include <iostream>

namespace eld::model {

namespace {

constexpr uint32_t OptionalChunkPresent = 1;
constexpr uint32_t PerFacePriority = 255;

constexpr uint8_t TriangleTypeNew = 1;
constexpr uint8_t TriangleTypeReuseCAsB = 2;
constexpr uint8_t TriangleTypeReuseCAsA = 3;
constexpr uint8_t TriangleTypeSwapAB = 4;

constexpr uint8_t TextureRenderTypeMask = 0x3;
constexpr uint8_t NoTextureInfo = 0;
constexpr uint8_t InvalidTextureInfo = 255;

}

FaceDecoder::FaceDecoder(const ModelFile& file)
    : file_(file),
      triangleDataBuffer_(file.payload),
      colorBuffer_(file.payload),
      priorityBuffer_(file.payload),
      alphaBuffer_(file.payload),
      textureBuffer_(file.payload)
{
    triangleDataBuffer_.setPosition(file_.layout.triangleDataOffset);
    colorBuffer_.setPosition(file_.layout.triangleColorsOffset);

    if (file_.footer.priorityFlag == PerFacePriority) {
        priorityBuffer_.setPosition(file_.layout.trianglePrioritiesOffset);
    }

    if (file_.footer.alphaFlag == OptionalChunkPresent) {
        alphaBuffer_.setPosition(file_.layout.triangleAlphasOffset);
    }

    if (file_.footer.textureFlag == OptionalChunkPresent) {
        textureBuffer_.setPosition(file_.layout.texturePointersOffset);
    }
}

std::vector<Face> FaceDecoder::decode() {
    std::vector<Face> faces;
    faces.reserve(file_.footer.triangleCount);

    for (uint32_t i = 0; i < file_.footer.triangleCount; i++) {
        faces.push_back(decodeFace(i));
    }

    return faces;
}

Face FaceDecoder::decodeFace(uint32_t index) {
    uint8_t triangleType =
        static_cast<uint8_t>(
            file_.payload[file_.layout.triangleTypesOffset + index]
        );

    decodeTriangleIndices(triangleType);

    FaceTextureInfo texture =
        decodeTextureInfo();

    Face face {};

    face.a = indexState_.a;
    face.b = indexState_.b;
    face.c = indexState_.c;

    face.color = decodeColor();
    face.priority = decodePriority();
    face.alpha = decodeAlpha();

    face.triangleType = triangleType;
    face.renderType = texture.renderType;
    face.texturePointer = texture.texturePointer;
    face.textureUVMappingIndex = texture.textureUVMappingIndex;

    return face;
}

void FaceDecoder::decodeTriangleIndices(uint8_t triangleType) {
    switch (triangleType) {
        case TriangleTypeNew:
            indexState_.a = readNextTriangleIndex();
            indexState_.b = readNextTriangleIndex();
            indexState_.c = readNextTriangleIndex();
            break;

        case TriangleTypeReuseCAsB:
            indexState_.b = indexState_.c;
            indexState_.c = readNextTriangleIndex();
            break;

        case TriangleTypeReuseCAsA:
            indexState_.a = indexState_.c;
            indexState_.c = readNextTriangleIndex();
            break;

        case TriangleTypeSwapAB:
            std::swap(indexState_.a, indexState_.b);
            indexState_.c = readNextTriangleIndex();
            break;

        default:
            break;
    }
}

int FaceDecoder::readNextTriangleIndex() {
    int index =
        triangleDataBuffer_.readSignedSmart() +
        indexState_.lastIndex;

    indexState_.lastIndex = index;

    return index;
}

uint16_t FaceDecoder::decodeColor() {
    return colorBuffer_.readU16();
}

uint8_t FaceDecoder::decodePriority() {
    if (file_.footer.priorityFlag == PerFacePriority) {
        return priorityBuffer_.readU8();
    }

    return static_cast<uint8_t>(file_.footer.priorityFlag);
}

uint8_t FaceDecoder::decodeAlpha() {
    if (file_.footer.alphaFlag == OptionalChunkPresent) {
        return alphaBuffer_.readU8();
    }

    return 0;
}

FaceTextureInfo FaceDecoder::decodeTextureInfo() {
    if (file_.footer.textureFlag != OptionalChunkPresent) {
        return {};
    }

    uint8_t value =
        textureBuffer_.readU8();

    if (
        value == NoTextureInfo ||
        value == InvalidTextureInfo
    ) {
        return {};
    }

    FaceTextureInfo texture {};

    texture.texturePointer = value;
    texture.renderType =
        static_cast<uint8_t>(
            value & TextureRenderTypeMask
        );
    texture.textureUVMappingIndex = value >> 2;

    return texture;
}

}
