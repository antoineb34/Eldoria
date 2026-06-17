#pragma once

#include <cstdint>
#include <vector>

#include "../ModelAsset.h"
#include "../ModelFile.h"
#include "binary/ByteReader.h"

namespace eld::model {

class VertexDecoder {
public:
    explicit VertexDecoder(const ModelFile& file);

    std::vector<Vertex> decode();

private:
    Vertex decodeVertex(uint32_t index);

    const ModelFile& file_;

    binary::ByteReader xBuffer_;
    binary::ByteReader yBuffer_;
    binary::ByteReader zBuffer_;

    int currentX_ = 0;
    int currentY_ = 0;
    int currentZ_ = 0;
};

}
