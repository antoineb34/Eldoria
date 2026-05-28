#pragma once

#include <cstdint>
#include <vector>

#include "ModelDef.h"
#include "ModelFooter.h"
#include "ModelLayout.h"

namespace rf::model {

std::vector<Face> decodeFaces(
    const std::vector<uint8_t>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
);

}
