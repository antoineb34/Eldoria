#pragma once

#include <string>

#include "interface/InterfaceRepository.h"

namespace eld::elforge {

class InterfaceInspector {
public:
    static std::string inspect(
        const eld::interface::InterfaceWidget& root,
        const eld::interface::InterfaceRepository& repository
    );
};

}
