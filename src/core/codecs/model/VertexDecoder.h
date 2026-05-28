#pragma once

#include <vector>
#include "ModelDef.h"

namespace rf::model {

std::vector<Vertex> decodeVertices(
    const std::vector<uint8_t>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
