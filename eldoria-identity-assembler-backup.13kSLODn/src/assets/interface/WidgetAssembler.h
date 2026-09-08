#pragma once

#include <cstdint>
#include <unordered_set>

#include "interface/WidgetData.h"
#include "interface/WidgetResource.h"

namespace eld::interface {

class WidgetLoader;

class WidgetAssembler {
public:
  explicit WidgetAssembler(const WidgetLoader &widgets);

  WidgetResource assemble(std::uint16_t rootId) const;

private:
  WidgetNode assembleNode(const WidgetData &data, std::int16_t x,
                          std::int16_t y,
                          std::unordered_set<std::uint16_t> &stack) const;

  const WidgetLoader &widgets_;
};

} // namespace eld::interface
