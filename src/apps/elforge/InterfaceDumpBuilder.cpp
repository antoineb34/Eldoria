#include "InterfaceDumpBuilder.h"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

namespace eld::elforge {

namespace {

void appendInterfaceDump(
    std::ostringstream& stream,
    const eld::interface::InterfaceWidget& widget,
    const eld::interface::InterfaceRepository& repository,
    int depth,
    int x,
    int y
) {
    const std::string indent(
        static_cast<std::size_t>(depth) * 2,
        ' '
    );

    stream
        << indent
        << "id=" << widget.id
        << " type=" << static_cast<int>(widget.type)
        << " action=" << static_cast<int>(widget.actionType)
        << " content=" << widget.contentType
        << " pos=" << x << "," << y
        << " size=" << widget.width << "x" << widget.height
        << " scroll=" << widget.scrollHeight
        << " hidden=" << (widget.hidden ? "yes" : "no")
        << " children=" << widget.children.size()
        << "\n";

    if (!widget.text.empty()) {
        stream << indent << "  text=\"" << widget.text << "\"\n";
    }

    if (!widget.secondaryText.empty()) {
        stream << indent << "  secondaryText=\"" << widget.secondaryText << "\"\n";
    }

    if (!widget.sprite.empty()) {
        stream << indent << "  sprite=\"" << widget.sprite << "\"\n";
    }

    if (!widget.secondarySprite.empty()) {
        stream << indent << "  secondarySprite=\"" << widget.secondarySprite << "\"\n";
    }

    if (widget.modelId.has_value()) {
        stream
            << indent
            << "  model=" << *widget.modelId
            << " zoom=" << widget.modelZoom
            << " rot=" << widget.modelRotationX
            << "," << widget.modelRotationY
            << "\n";
    }

    if (widget.secondaryModelId.has_value()) {
        stream << indent << "  secondaryModel=" << *widget.secondaryModelId << "\n";
    }

    if (widget.animationId.has_value()) {
        stream << indent << "  animation=" << *widget.animationId << "\n";
    }

    if (widget.secondaryAnimationId.has_value()) {
        stream << indent << "  secondaryAnimation=" << *widget.secondaryAnimationId << "\n";
    }

    if (!widget.itemIds.empty()) {
        std::size_t nonEmpty = 0;

        for (const std::uint16_t itemId : widget.itemIds) {
            if (itemId != 0) {
                ++nonEmpty;
            }
        }

        stream
            << indent
            << "  items=" << widget.itemIds.size()
            << " nonEmpty=" << nonEmpty
            << " padding=" << static_cast<int>(widget.inventoryPaddingX)
            << "," << static_cast<int>(widget.inventoryPaddingY)
            << "\n";
    }

    if (!widget.inventorySprites.empty()) {
        stream
            << indent
            << "  inventorySprites=" << widget.inventorySprites.size()
            << "\n";

        for (const eld::interface::InterfaceFileSpriteSlot& slot : widget.inventorySprites) {
            stream
                << indent
                << "    slot=" << static_cast<int>(slot.slot)
                << " pos=" << slot.x << "," << slot.y
                << " sprite=\"" << slot.sprite << "\"\n";
        }
    }

    for (const eld::interface::InterfaceFileChild& child : widget.children) {
        const eld::interface::InterfaceWidget* childWidget =
            repository.find(child.id);

        if (childWidget == nullptr) {
            stream
                << indent
                << "  missing-child id=" << child.id
                << " pos=" << child.x << "," << child.y
                << "\n";

            continue;
        }

        appendInterfaceDump(
            stream,
            *childWidget,
            repository,
            depth + 1,
            x + child.x,
            y + child.y
        );
    }
}

}

std::string InterfaceDumpBuilder::build(
    const eld::interface::InterfaceWidget& root,
    const eld::interface::InterfaceRepository& repository
) {
    std::ostringstream stream;

    stream << "Interface subtree dump\n";
    stream << "======================\n";

    appendInterfaceDump(
        stream,
        root,
        repository,
        0,
        0,
        0
    );

    return stream.str();
}

}
