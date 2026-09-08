#pragma once

#include "interface/WidgetData.h"
#include "interface/WidgetResource.h"

namespace eld::interface {

class WidgetAssembler {
public:
  WidgetResource assemble(WidgetData data) const;
};

} // namespace eld::interface
