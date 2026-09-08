#pragma once

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

#include "interface/WidgetData.h"
#include "interface/WidgetLoader.h"

namespace eld::interface {

struct InterfaceNode {
  Widget widget;

  std::int16_t x = 0;
  std::int16_t y = 0;

  std::vector<InterfaceNode> children;
};

struct ComposedInterface {
  std::uint16_t rootId = 0;
  InterfaceNode root;
};

class InterfaceComposer {
public:
  explicit InterfaceComposer(const WidgetLoader &widgets);

  std::optional<ComposedInterface> compose(std::uint16_t rootId) const;

private:
  InterfaceNode composeNode(const Widget &widget, std::int16_t x,
                            std::int16_t y,
                            std::unordered_set<std::uint16_t> &stack) const;

  const WidgetLoader &widgets_;
};

} // namespace eld::interface
