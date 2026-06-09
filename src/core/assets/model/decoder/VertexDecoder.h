#pragma once

#include <cstdint>
#include <vector>

#include "../ModelAsset.h"
#include "../ModelFile.h"
#include "binary/ByteBuffer.h"

namespace rf::model {

class VertexDecoder {
public:
    explicit VertexDecoder(const ModelFile& file);

    std::vector<Vertex> decode();

private:
    Vertex decodeVertex(uint32_t index);

    const ModelFile& file_;

    rf::io::ByteBuffer xBuffer_;
    rf::io::ByteBuffer yBuffer_;
    rf::io::ByteBuffer zBuffer_;

    int currentX_ = 0;
    int currentY_ = 0;
    int currentZ_ = 0;
};

}