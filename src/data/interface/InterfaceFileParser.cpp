#include "InterfaceFileParser.h"

#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::interface {

namespace {

std::optional<std::uint16_t> readReference(
    eld::binary::ByteReader& reader
) {
    const std::uint8_t high =
        reader.readU8();

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
    const std::uint16_t value =
        reader.readU16();

    return value == 65535
        ? -1
        : static_cast<std::int32_t>(value);
}

}

std::optional<InterfaceFile>
InterfaceFileParser::parse(
    const std::vector<std::uint8_t>& payload
) const {
    try {
        eld::binary::ByteReader reader(payload);

        InterfaceFile file;
        file.payload = payload;
        file.declaredCount =
            reader.readU16();

        std::vector<InterfaceFileWidget> widgets;
        widgets.reserve(file.declaredCount);

        std::optional<std::uint16_t> parentId;

        while (!reader.atEnd()) {
            std::uint16_t id =
                reader.readU16();

            if (id == 65535) {
                parentId =
                    reader.readU16();

                id =
                    reader.readU16();
            }

            InterfaceFileWidget definition;
            definition.id = id;
            definition.parentId = parentId;

            definition.type = reader.readU8();
            definition.actionType = reader.readU8();
            definition.contentType = reader.readU16();
            definition.width = reader.readU16();
            definition.height = reader.readU16();
            definition.opacity = reader.readU8();
            definition.hoverId = readReference(reader);

            const std::uint8_t conditionCount =
                reader.readU8();

            definition.conditions.reserve(
                conditionCount
            );

            for (
                std::uint8_t index = 0;
                index < conditionCount;
                ++index
            ) {
                definition.conditions.push_back({
                    reader.readU8(),
                    reader.readU16()
                });
            }

            const std::uint8_t scriptCount =
                reader.readU8();

            definition.scripts.reserve(scriptCount);

            for (
                std::uint8_t index = 0;
                index < scriptCount;
                ++index
            ) {
                const std::uint16_t length =
                    reader.readU16();

                InterfaceFileScript script;
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

                definition.scripts.push_back(
                    std::move(script)
                );
            }

            if (definition.type == 0) {
                definition.scrollHeight =
                    reader.readU16();

                definition.hidden =
                    reader.readU8() == 1;

                const std::uint16_t childCount =
                    reader.readU16();

                definition.children.reserve(
                    childCount
                );

                for (
                    std::uint16_t index = 0;
                    index < childCount;
                    ++index
                ) {
                    definition.children.push_back({
                        reader.readU16(),
                        reader.readI16(),
                        reader.readI16()
                    });
                }
            }

            if (definition.type == 1) {
                reader.readU16();
                reader.readU8();
            }

            if (
                definition.type == 2 ||
                definition.type == 7
            ) {
                const std::size_t slotCount =
                    static_cast<std::size_t>(
                        definition.width
                    ) *
                    definition.height;

                definition.itemIds.resize(slotCount);
                definition.itemAmounts.resize(slotCount);
            }

            if (definition.type == 2) {
                definition.inventorySwap =
                    reader.readU8() == 1;

                definition.inventoryInterface =
                    reader.readU8() == 1;

                definition.inventoryUsable =
                    reader.readU8() == 1;

                definition.inventoryReplace =
                    reader.readU8() == 1;

                definition.inventoryPaddingX =
                    reader.readU8();

                definition.inventoryPaddingY =
                    reader.readU8();

                for (
                    std::uint8_t slot = 0;
                    slot < 20;
                    ++slot
                ) {
                    if (reader.readU8() == 1) {
                        definition.inventorySprites.push_back({
                            slot,
                            reader.readI16(),
                            reader.readI16(),
                            reader.readTerminatedString(10)
                        });
                    }
                }

                definition.actions.reserve(5);

                for (
                    std::uint8_t index = 0;
                    index < 5;
                    ++index
                ) {
                    definition.actions.push_back(
                        reader.readTerminatedString(10)
                    );
                }
            }

            if (definition.type == 3) {
                definition.filled =
                    reader.readU8() == 1;
            }

            if (
                definition.type == 1 ||
                definition.type == 4
            ) {
                definition.centeredText =
                    reader.readU8() == 1;

                definition.fontId =
                    reader.readU8();

                definition.textShadow =
                    reader.readU8() == 1;
            }

            if (definition.type == 4) {
                definition.text =
                    reader.readTerminatedString(10);

                definition.secondaryText =
                    reader.readTerminatedString(10);
            }

            if (
                definition.type == 1 ||
                definition.type == 3 ||
                definition.type == 4
            ) {
                definition.color =
                    reader.readU32();
            }

            if (
                definition.type == 3 ||
                definition.type == 4
            ) {
                definition.secondaryColor =
                    reader.readU32();

                definition.hoverColor =
                    reader.readU32();

                definition.secondaryHoverColor =
                    reader.readU32();
            }

            if (definition.type == 5) {
                definition.sprite =
                    reader.readTerminatedString(10);

                definition.secondarySprite =
                    reader.readTerminatedString(10);
            }

            if (definition.type == 6) {
                definition.modelId =
                    readReference(reader);

                definition.secondaryModelId =
                    readReference(reader);

                definition.animationId =
                    readReference(reader);

                definition.secondaryAnimationId =
                    readReference(reader);

                definition.modelZoom =
                    reader.readU16();

                definition.modelRotationX =
                    reader.readU16();

                definition.modelRotationY =
                    reader.readU16();
            }

            if (definition.type == 7) {
                definition.centeredText =
                    reader.readU8() == 1;

                definition.fontId =
                    reader.readU8();

                definition.textShadow =
                    reader.readU8() == 1;

                definition.color =
                    reader.readU32();

                definition.itemPaddingX =
                    reader.readI16();

                definition.itemPaddingY =
                    reader.readI16();

                definition.inventoryInterface =
                    reader.readU8() == 1;

                definition.actions.reserve(5);

                for (
                    std::uint8_t index = 0;
                    index < 5;
                    ++index
                ) {
                    definition.actions.push_back(
                        reader.readTerminatedString(10)
                    );
                }
            }

            if (definition.type == 8) {
                definition.text =
                    reader.readTerminatedString(10);
            }

            if (
                definition.actionType == 2 ||
                definition.type == 2
            ) {
                definition.selectedAction =
                    reader.readTerminatedString(10);

                definition.spellName =
                    reader.readTerminatedString(10);

                definition.spellTargets =
                    reader.readU16();
            }

            if (
                definition.actionType == 1 ||
                definition.actionType == 4 ||
                definition.actionType == 5 ||
                definition.actionType == 6
            ) {
                definition.tooltip =
                    reader.readTerminatedString(10);

                if (definition.tooltip.empty()) {
                    if (definition.actionType == 1) {
                        definition.tooltip = "Ok";
                    }
                    else if (
                        definition.actionType == 4 ||
                        definition.actionType == 5
                    ) {
                        definition.tooltip = "Select";
                    }
                    else {
                        definition.tooltip = "Continue";
                    }
                }
            }

            widgets.push_back(
                std::move(definition)
            );
        }

        file.widgets = std::move(widgets);
        return file;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
