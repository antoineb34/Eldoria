#include "ModelFooter.h"

#include <iostream>

namespace rf::model {

static constexpr int MODEL_FOOTER_SIZE = 18;

ModelFooter readModelFooter(
    const std::vector<uint8_t>& payload
) {

    if (payload.size() < MODEL_FOOTER_SIZE) {

        std::cerr
            << "Payload too small for model footer\n";

        return {};
    }

    int footerStart =
        payload.size() -
        MODEL_FOOTER_SIZE;

    ModelFooter footer {};

    footer.vertexCount =
        ((unsigned char)payload[footerStart + 0] << 8) |
        ((unsigned char)payload[footerStart + 1]);

    footer.triangleCount =
        ((unsigned char)payload[footerStart + 2] << 8) |
        ((unsigned char)payload[footerStart + 3]);

    footer.textureTriangleCount =
        (unsigned char)payload[footerStart + 4];

    footer.textureFlag =
        (unsigned char)payload[footerStart + 5];

    footer.priorityFlag =
        (unsigned char)payload[footerStart + 6];

    footer.alphaFlag =
        (unsigned char)payload[footerStart + 7];

    footer.triangleSkinFlag =
        (unsigned char)payload[footerStart + 8];

    footer.vertexSkinFlag =
        (unsigned char)payload[footerStart + 9];

    footer.xDataLength =
        ((unsigned char)payload[footerStart + 10] << 8) |
        ((unsigned char)payload[footerStart + 11]);

    footer.yDataLength =
        ((unsigned char)payload[footerStart + 12] << 8) |
        ((unsigned char)payload[footerStart + 13]);

    footer.zDataLength =
        ((unsigned char)payload[footerStart + 14] << 8) |
        ((unsigned char)payload[footerStart + 15]);

    footer.triangleDataLength =
        ((unsigned char)payload[footerStart + 16] << 8) |
        ((unsigned char)payload[footerStart + 17]);

    return footer;
}

}
