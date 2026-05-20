#pragma once

#include <vector>

#include "../io/ByteBuffer.h"
#include "ModelFooter.h"
#include "ModelLayout.h"

namespace rf::model {

struct Vertex {
    int x = 0;
    int y = 0;
    int z = 0;
};

std::vector<Vertex> decodeVertices(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
