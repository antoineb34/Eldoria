#pragma once

#include "ModelFooter.h"

namespace rf::model {

struct ModelLayout {

    int vertexFlagsOffset;

    int triangleTypesOffset;
    int trianglePrioritiesOffset;
    int triangleSkinsOffset;

    int texturePointersOffset;

    int vertexSkinsOffset;
    int triangleAlphasOffset;

    int triangleDataOffset;
    int triangleColorsOffset;

    int textureDataOffset;

    int xDataOffset;
    int yDataOffset;
    int zDataOffset;
};

ModelLayout calculateModelLayout(
    const ModelFooter& footer
);

}
