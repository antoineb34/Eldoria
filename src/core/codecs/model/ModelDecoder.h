#pragma once

#include <vector>
#include "ModelDef.h"

namespace rf::model {

ModelDef decodeModel(
    const std::vector<char>& payload
);

}
