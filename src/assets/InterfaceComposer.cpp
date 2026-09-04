#include "InterfaceComposer.h"

#include <cstdint>
#include <optional>
#include <unordered_set>

namespace eld::interface {

InterfaceComposer::InterfaceComposer(
    const WidgetRepository& widgets
)
    : widgets_(widgets) {
}


std::optional<ComposedInterface>
InterfaceComposer::compose(
    std::uint16_t rootId
) const {
    const Widget* root =
        widgets_.find(rootId);

    if (root == nullptr) {
        return std::nullopt;
    }

    std::unordered_set<std::uint16_t> stack;

    return ComposedInterface{
        .rootId = rootId,
        .root = composeNode(
            *root,
            0,
            0,
            stack
        )
    };
}


InterfaceNode InterfaceComposer::composeNode(
    const Widget& widget,
    std::int16_t x,
    std::int16_t y,
    std::unordered_set<std::uint16_t>& stack
) const {
    stack.insert(widget.id);

    InterfaceNode node{
        .widget = widget,
        .x = x,
        .y = y
    };

    node.children.reserve(
        widget.children.size()
    );


    // Children

    for (
        const WidgetChild& child :
        widget.children
    ) {
        if (stack.contains(child.id)) {
            continue;
        }

        const Widget* childWidget =
            widgets_.find(child.id);

        if (childWidget == nullptr) {
            continue;
        }

        node.children.push_back(
            composeNode(
                *childWidget,
                child.x,
                child.y,
                stack
            )
        );
    }

    stack.erase(widget.id);

    return node;
}

}
