#pragma once

#include <string>

#include "interface/InterfaceRepository.h"

namespace eld::elforge {

class InterfaceDumpBuilder {
public:
    static std::string build(
        const eld::interface::InterfaceWidget& root,
        const eld::interface::InterfaceRepository& repository
    );
};

}
