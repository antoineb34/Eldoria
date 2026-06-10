#pragma once

#include <cstdint>
#include <vector>

#include "../ModelAsset.h"
#include "../ModelFile.h"
#include "binary/ByteBuffer.h"

namespace eld::model {

struct TriangleIndexState {
    int a = 0;
    int b = 0;
    int c = 0;
    int lastIndex = 0;
};

struct FaceTextureInfo {
    int texturePointer = -1;
    uint8_t renderType = 0;
    int textureUVMappingIndex = -1;
};

class FaceDecoder {
public:
    explicit FaceDecoder(
        const ModelFile& file
    );

    std::vector<Face> decode();

private:
    Face decodeFace(
        uint32_t index
    );

    void decodeTriangleIndices(
        uint8_t triangleType
    );

    int readNextTriangleIndex();

    uint16_t decodeColor();
    uint8_t decodePriority();
    uint8_t decodeAlpha();

    FaceTextureInfo decodeTextureInfo();

    const ModelFile& file_;

    binary::ByteBuffer triangleDataBuffer_;
    binary::ByteBuffer colorBuffer_;
    binary::ByteBuffer priorityBuffer_;
    binary::ByteBuffer alphaBuffer_;
    binary::ByteBuffer textureBuffer_;

    TriangleIndexState indexState_;
};

}
