#include "FaceDecoder.h"
#include "../io/ByteBuffer.h"

namespace rf::model {

std::vector<Face> decodeFaces(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
) {

    rf::io::ByteBuffer textureBuffer(payload);

    if (footer.textureFlag == 1) {

        textureBuffer.setPosition(
            layout.texturePointersOffset
        );
    }

    rf::io::ByteBuffer priorityBuffer(payload);

    rf::io::ByteBuffer alphaBuffer(payload);

    if (footer.alphaFlag == 1) {
        alphaBuffer.setPosition(
            layout.triangleAlphasOffset
        );
    }

    if (footer.priorityFlag == 255) {

        priorityBuffer.setPosition(
            layout.trianglePrioritiesOffset
        );
    }

    rf::io::ByteBuffer colorBuffer(payload);

    colorBuffer.setPosition(
        layout.triangleColorsOffset
    );

    rf::io::ByteBuffer triangleDataBuffer(
        payload
    );

    triangleDataBuffer.setPosition(
        layout.triangleDataOffset
    );

    std::vector<Face> faces;

    faces.reserve(
        footer.triangleCount
    );

    int lastA = 0;
    int lastB = 0;
    int lastC = 0;
    int lastIndex = 0;

    for (
        int i = 0;
        i < footer.triangleCount;
        i++
    ) {

        unsigned char type =
            static_cast<unsigned char>(
                payload[
                    layout.triangleTypesOffset + i
                ]
            );

        if (type == 1) {

            lastA =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastA;

            lastB =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastB;

            lastC =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastC;
        }

        else if (type == 2) {

            lastB = lastC;

            lastC =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastC;
        }

        else if (type == 3) {

            lastA = lastC;

            lastC =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastC;
        }

        else if (type == 4) {

            int oldA = lastA;

            lastA = lastB;
            lastB = oldA;

            lastC =
                triangleDataBuffer.readSmart() +
                lastIndex;

            lastIndex = lastC;
        }

        uint16_t color =
            colorBuffer.readU16();

        uint8_t priority = 0;

        if (footer.priorityFlag == 255) {
            priority = priorityBuffer.readU8();
        }
        else {
            priority = footer.priorityFlag;
        }

        uint8_t alpha = 0;

        if (footer.alphaFlag == 1) {
            alpha = alphaBuffer.readU8();
        }

        int textureFlag = -1;

        if (footer.textureFlag == 1) {

            uint8_t value =
                textureBuffer.readU8();

            if (value != 255) {
                textureFlag = value;
            }
        }

        faces.push_back({
            lastA,
            lastB,
            lastC,
            color,
            priority,
            alpha,
            textureFlag
        });
    }

    return faces;
}

}
