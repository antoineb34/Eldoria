#pragma once

#include <string>

#include "repositories/WidgetRepository.h"

namespace eld::elforge {

class InterfaceInspector {
public:
    static std::string inspect(
        const eld::interface::Widget& root,
        const eld::interface::WidgetRepository& repository
    );
};

}
