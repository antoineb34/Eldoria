#include "VertexDecoder.h"

#include "../../io/ByteBuffer.h"

namespace rf::model {

std::vector<Vertex> decodeVertices(
    const std::vector<uint8_t>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
) {

    rf::io::ByteBuffer xBuffer(payload);
    rf::io::ByteBuffer yBuffer(payload);
    rf::io::ByteBuffer zBuffer(payload);

    xBuffer.setPosition(
        layout.xDataOffset
    );

    yBuffer.setPosition(
        layout.yDataOffset
    );

    zBuffer.setPosition(
        layout.zDataOffset
    );

    std::vector<Vertex> vertices;

    vertices.reserve(
        footer.vertexCount
    );

    int currentX = 0;
    int currentY = 0;
    int currentZ = 0;

    for (
        int i = 0;
        i < footer.vertexCount;
        i++
    ) {

        unsigned char flag =
            static_cast<unsigned char>(
                payload[
                    layout.vertexFlagsOffset + i
                ]
            );

        int dx = 0;
        int dy = 0;
        int dz = 0;

        if (flag & 1) {
            dx = xBuffer.readSmart();
        }

        if (flag & 2) {
            dy = yBuffer.readSmart();
        }

        if (flag & 4) {
            dz = zBuffer.readSmart();
        }

        currentX += dx;
        currentY += dy;
        currentZ += dz;

        vertices.push_back({
            currentX,
            currentY,
            currentZ
        });
    }

    return vertices;
}

}
