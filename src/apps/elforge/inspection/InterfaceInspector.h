#pragma once

#include <string>

#include "interface/WidgetLoader.h"

namespace eld::elforge {

class InterfaceInspector {
public:
  static std::string inspect(const eld::interface::WidgetData &root,
                             const eld::interface::WidgetLoader &repository);
};

} // namespace eld::elforge
