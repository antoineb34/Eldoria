#pragma once

#include <vector>

#include "ModelFooter.h"
#include "ModelLayout.h"

namespace rf::model {

struct Face {
    int a = 0;
    int b = 0;
    int c = 0;

    uint16_t color = 0;
    uint8_t priority = 0;
};

std::vector<Face> decodeFaces(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
