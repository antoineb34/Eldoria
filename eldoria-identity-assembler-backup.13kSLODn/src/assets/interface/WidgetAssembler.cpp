#include "interface/WidgetAssembler.h"

#include <stdexcept>

#include "interface/WidgetLoader.h"

namespace eld::interface {

WidgetAssembler::WidgetAssembler(const WidgetLoader &widgets)
    : widgets_(widgets) {}

WidgetResource WidgetAssembler::assemble(std::uint16_t rootId) const {
  if (!widgets_.contains(rootId)) {
    throw std::out_of_range("Interface root widget does not exist");
  }

  std::unordered_set<std::uint16_t> stack;

  return WidgetResource{
      .rootId = rootId,
      .root = assembleNode(widgets_.data(rootId), 0, 0, stack),
  };
}

WidgetNode
WidgetAssembler::assembleNode(const WidgetData &data, std::int16_t x,
                              std::int16_t y,
                              std::unordered_set<std::uint16_t> &stack) const {
  stack.insert(data.id);

  WidgetNode node{
      .data = data,
      .x = x,
      .y = y,
  };

  node.children.reserve(data.children.size());

  for (const WidgetChild &child : data.children) {
    if (stack.contains(child.id) || !widgets_.contains(child.id)) {
      continue;
    }

    node.children.push_back(
        assembleNode(widgets_.data(child.id), child.x, child.y, stack));
  }

  stack.erase(data.id);

  return node;
}

} // namespace eld::interface
