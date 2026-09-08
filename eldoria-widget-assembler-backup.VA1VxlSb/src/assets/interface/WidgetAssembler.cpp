#include "interface/WidgetAssembler.h"

#include <utility>

namespace eld::interface {

WidgetResource WidgetAssembler::assemble(WidgetData data) const {
  return WidgetResource{
      .data = std::move(data),
  };
}

} // namespace eld::interface
