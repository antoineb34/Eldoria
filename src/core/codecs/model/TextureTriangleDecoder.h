#pragma once

#include <vector>
#include "ModelDef.h"

namespace rf::model {

std::vector<TextureTriangle> decodeTextureTriangles(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
