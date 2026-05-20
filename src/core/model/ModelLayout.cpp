#include "ModelLayout.h"

namespace rf::model {

ModelLayout calculateModelLayout(
    const ModelFooter& footer
) {

    ModelLayout layout {};

    int offset = 0;

    layout.vertexFlagsOffset = offset;
    offset += footer.vertexCount;

    layout.triangleTypesOffset = offset;
    offset += footer.triangleCount;

    layout.trianglePrioritiesOffset = offset;

    if (footer.priorityFlag == 255) {
        offset += footer.triangleCount;
    }

    layout.triangleSkinsOffset = offset;

    if (footer.triangleSkinFlag == 1) {
        offset += footer.triangleCount;
    }

    layout.texturePointersOffset = offset;

    if (footer.textureFlag == 1) {
        offset += footer.triangleCount;
    }

    layout.vertexSkinsOffset = offset;

    if (footer.vertexSkinFlag == 1) {
        offset += footer.vertexCount;
    }

    layout.triangleAlphasOffset = offset;

    if (footer.alphaFlag == 1) {
        offset += footer.triangleCount;
    }

    layout.triangleDataOffset = offset;
    offset += footer.triangleDataLength;

    layout.triangleColorsOffset = offset;
    offset += footer.triangleCount * 2;

    layout.textureDataOffset = offset;
    offset += footer.textureTriangleCount * 6;

    layout.xDataOffset = offset;
    offset += footer.xDataLength;

    layout.yDataOffset = offset;
    offset += footer.yDataLength;

    layout.zDataOffset = offset;
    offset += footer.zDataLength;

    return layout;
}

}
