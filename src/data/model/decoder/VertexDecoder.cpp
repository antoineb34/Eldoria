#include "VertexDecoder.h"

namespace rf::model {

namespace {

constexpr uint8_t VertexHasXDelta = 1;
constexpr uint8_t VertexHasYDelta = 2;
constexpr uint8_t VertexHasZDelta = 4;

}

VertexDecoder::VertexDecoder(const ModelFile& file)
    : file_(file),
      xBuffer_(file.payload),
      yBuffer_(file.payload),
      zBuffer_(file.payload)
{
    xBuffer_.setPosition(file_.layout.xDataOffset);
    yBuffer_.setPosition(file_.layout.yDataOffset);
    zBuffer_.setPosition(file_.layout.zDataOffset);
}

std::vector<Vertex> VertexDecoder::decode() {
    std::vector<Vertex> vertices;
    vertices.reserve(file_.footer.vertexCount);

    for (uint32_t i = 0; i < file_.footer.vertexCount; i++) {
        vertices.push_back(decodeVertex(i));
    }

    return vertices;
}

Vertex VertexDecoder::decodeVertex(uint32_t index) {
    uint8_t flag =
        file_.payload[file_.layout.vertexFlagsOffset + index];

    int dx = 0;
    int dy = 0;
    int dz = 0;

    if (flag & VertexHasXDelta) {
        dx = xBuffer_.readSignedSmart();
    }

    if (flag & VertexHasYDelta) {
        dy = yBuffer_.readSignedSmart();
    }

    if (flag & VertexHasZDelta) {
        dz = zBuffer_.readSignedSmart();
    }

    currentX_ += dx;
    currentY_ += dy;
    currentZ_ += dz;

    return {
        currentX_,
        currentY_,
        currentZ_
    };
}

}
