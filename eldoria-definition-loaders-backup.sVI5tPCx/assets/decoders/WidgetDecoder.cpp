#include "decoders/WidgetDecoder.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::interface {

namespace {

std::optional<std::uint16_t> readReference(
    eld::binary::ByteReader& reader
) {
    const auto high = reader.readU8();

    if (high == 0) {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high - 1) << 8) |
        reader.readU8()
    );
}


std::int32_t readScriptInstruction(
    eld::binary::ByteReader& reader
) {
    const auto value = reader.readU16();

    return
        value == 65535
            ? -1
            : static_cast<std::int32_t>(value);
}


void readTextSettings(
    eld::binary::ByteReader& reader,
    Widget& widget
) {
    widget.centeredText = reader.readU8() == 1;
    widget.fontId = reader.readU8();
    widget.textShadow = reader.readU8() == 1;
}


void readActions(
    eld::binary::ByteReader& reader,
    Widget& widget
) {
    widget.actions.reserve(5);

    for (
        std::uint8_t index = 0;
        index < 5;
        ++index
    ) {
        widget.actions.push_back(
            reader.readTerminatedString(10)
        );
    }
}


void resizeItemSlots(
    Widget& widget
) {
    const std::size_t slotCount =
        static_cast<std::size_t>(widget.width) *
        widget.height;

    widget.itemIds.resize(slotCount);
    widget.itemAmounts.resize(slotCount);
}

}


std::vector<Widget> WidgetDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    const auto tableSize = reader.readU16();

    std::vector<Widget> widgets;
    widgets.reserve(tableSize);

    std::optional<std::uint16_t> parentId;


    // Widgets

    while (!reader.atEnd()) {
        auto id = reader.readU16();

        if (id == 65535) {
            parentId = reader.readU16();
            id = reader.readU16();
        }

        if (id >= tableSize) {
            throw std::runtime_error(
                "Widget id exceeds table"
            );
        }

        Widget widget;

        widget.id = id;
        widget.parentId = parentId;

        widget.type = reader.readU8();
        widget.actionType = reader.readU8();
        widget.contentType = reader.readU16();

        widget.width = reader.readU16();
        widget.height = reader.readU16();

        widget.opacity = reader.readU8();
        widget.hoverId = readReference(reader);


        // Conditions

        const auto conditionCount = reader.readU8();

        widget.conditions.reserve(conditionCount);

        for (
            std::uint8_t index = 0;
            index < conditionCount;
            ++index
        ) {
            widget.conditions.push_back({
                reader.readU8(),
                reader.readU16()
            });
        }


        // Scripts

        const auto scriptCount = reader.readU8();

        widget.scripts.reserve(scriptCount);

        for (
            std::uint8_t index = 0;
            index < scriptCount;
            ++index
        ) {
            const auto length = reader.readU16();

            WidgetScript script;
            script.instructions.reserve(length);

            for (
                std::uint16_t instruction = 0;
                instruction < length;
                ++instruction
            ) {
                script.instructions.push_back(
                    readScriptInstruction(reader)
                );
            }

            widget.scripts.push_back(
                std::move(script)
            );
        }


        // Type

        switch (widget.type) {
            case 0: {
                // Container

                widget.scrollHeight = reader.readU16();
                widget.hidden = reader.readU8() == 1;

                const auto childCount = reader.readU16();

                widget.children.reserve(childCount);

                for (
                    std::uint16_t index = 0;
                    index < childCount;
                    ++index
                ) {
                    widget.children.push_back({
                        reader.readU16(),
                        reader.readI16(),
                        reader.readI16()
                    });
                }

                break;
            }

            case 1:
                // Legacy text

                reader.readU16();
                reader.readU8();

                readTextSettings(
                    reader,
                    widget
                );

                widget.color = reader.readU32();
                break;

            case 2:
                // Inventory

                resizeItemSlots(widget);

                widget.inventorySwap =
                    reader.readU8() == 1;

                widget.inventoryInterface =
                    reader.readU8() == 1;

                widget.inventoryUsable =
                    reader.readU8() == 1;

                widget.inventoryReplace =
                    reader.readU8() == 1;

                widget.inventoryPaddingX =
                    reader.readU8();

                widget.inventoryPaddingY =
                    reader.readU8();

                for (
                    std::uint8_t slot = 0;
                    slot < 20;
                    ++slot
                ) {
                    if (reader.readU8() != 1) {
                        continue;
                    }

                    widget.inventorySprites.push_back({
                        slot,
                        reader.readI16(),
                        reader.readI16(),
                        reader.readTerminatedString(10)
                    });
                }

                readActions(
                    reader,
                    widget
                );

                break;

            case 3:
                // Rectangle

                widget.filled = reader.readU8() == 1;

                widget.color = reader.readU32();
                widget.secondaryColor = reader.readU32();
                widget.hoverColor = reader.readU32();
                widget.secondaryHoverColor = reader.readU32();

                break;

            case 4:
                // Text

                readTextSettings(
                    reader,
                    widget
                );

                widget.text =
                    reader.readTerminatedString(10);

                widget.secondaryText =
                    reader.readTerminatedString(10);

                widget.color = reader.readU32();
                widget.secondaryColor = reader.readU32();
                widget.hoverColor = reader.readU32();
                widget.secondaryHoverColor = reader.readU32();

                break;

            case 5:
                // Sprite

                widget.sprite =
                    reader.readTerminatedString(10);

                widget.secondarySprite =
                    reader.readTerminatedString(10);

                break;

            case 6:
                // Model

                widget.modelId =
                    readReference(reader);

                widget.secondaryModelId =
                    readReference(reader);

                widget.animationId =
                    readReference(reader);

                widget.secondaryAnimationId =
                    readReference(reader);

                widget.modelZoom = reader.readU16();
                widget.modelRotationX = reader.readU16();
                widget.modelRotationY = reader.readU16();

                break;

            case 7:
                // Item list

                resizeItemSlots(widget);

                readTextSettings(
                    reader,
                    widget
                );

                widget.color = reader.readU32();

                widget.itemPaddingX = reader.readI16();
                widget.itemPaddingY = reader.readI16();

                widget.inventoryInterface =
                    reader.readU8() == 1;

                readActions(
                    reader,
                    widget
                );

                break;

            case 8:
                // Tooltip

                widget.text =
                    reader.readTerminatedString(10);

                break;

            default:
                throw std::runtime_error(
                    "Unknown widget type"
                );
        }


        // Spell

        if (
            widget.actionType == 2 ||
            widget.type == 2
        ) {
            widget.selectedAction =
                reader.readTerminatedString(10);

            widget.spellName =
                reader.readTerminatedString(10);

            widget.spellTargets = reader.readU16();
        }


        // Action

        if (
            widget.actionType == 1 ||
            widget.actionType == 4 ||
            widget.actionType == 5 ||
            widget.actionType == 6
        ) {
            widget.tooltip =
                reader.readTerminatedString(10);

            if (widget.tooltip.empty()) {
                if (widget.actionType == 1) {
                    widget.tooltip = "Ok";
                }
                else if (
                    widget.actionType == 4 ||
                    widget.actionType == 5
                ) {
                    widget.tooltip = "Select";
                }
                else {
                    widget.tooltip = "Continue";
                }
            }
        }

        widgets.push_back(
            std::move(widget)
        );
    }

    return widgets;
}

}
