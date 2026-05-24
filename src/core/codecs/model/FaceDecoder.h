#pragma once

#include <cstdint>
#include <vector>

#include "ModelDef.h"

namespace rf::model {

int findMatchingTextureTriangle(
    const Face& face,
    const std::vector<TextureTriangle>& textureTriangles
);

std::vector<Face> decodeFaces(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
