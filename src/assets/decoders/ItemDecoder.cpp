#include "decoders/ItemDecoder.h"

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::item {

namespace {

std::optional<std::uint16_t> readOptionalId(
    eld::binary::ByteReader& reader
) {
    const auto id = reader.readU16();

    return
        id == 65535
            ? std::nullopt
            : std::optional{id};
}


std::optional<ItemWearPosition> readWearPosition(
    eld::binary::ByteReader& reader
) {
    const auto value = reader.readU8();

    if (
        value >
        static_cast<std::uint8_t>(
            ItemWearPosition::Quiver
        )
    ) {
        return std::nullopt;
    }

    return static_cast<ItemWearPosition>(
        value
    );
}

}


Item ItemDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Item item;


    // Opcodes

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Invalid item payload"
                    );
                }

                return item;

            case 1:
                item.inventoryModelId =
                    readOptionalId(reader);
                break;

            case 2:
                item.name =
                    reader.readTerminatedString(10);
                break;

            case 3:
                item.description =
                    reader.readTerminatedString(10);
                break;

            case 4:
                item.zoom = reader.readU16();
                break;

            case 5:
                item.rotationX = reader.readU16();
                break;

            case 6:
                item.rotationY = reader.readU16();
                break;

            case 7:
                item.offsetX = reader.readI16();
                break;

            case 8:
                item.offsetY = reader.readI16();
                break;

            case 10:
                reader.readU16();
                break;

            case 11:
                item.stackable = true;
                break;

            case 12:
                item.value = reader.readU32();
                break;

            case 13:
                item.wearPosition =
                    readWearPosition(reader);
                break;

            case 14:
                item.wearPosition2 =
                    readWearPosition(reader);
                break;

            case 16:
                item.membersOnly = true;
                break;

            case 23:
                item.maleModelIds[0] =
                    readOptionalId(reader);

                item.maleModelOffset =
                    reader.readI8();
                break;

            case 24:
                item.maleModelIds[1] =
                    readOptionalId(reader);
                break;

            case 25:
                item.femaleModelIds[0] =
                    readOptionalId(reader);

                item.femaleModelOffset =
                    reader.readI8();
                break;

            case 26:
                item.femaleModelIds[1] =
                    readOptionalId(reader);
                break;

            case 27:
                item.wearPosition3 =
                    readWearPosition(reader);
                break;

            case 40: {
                const auto count = reader.readU8();

                item.recolors.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    item.recolors.push_back({
                        reader.readU16(),
                        reader.readU16()
                    });
                }

                break;
            }

            case 78:
                item.maleModelIds[2] =
                    readOptionalId(reader);
                break;

            case 79:
                item.femaleModelIds[2] =
                    readOptionalId(reader);
                break;

            case 90:
                item.maleHeadModelIds[0] =
                    readOptionalId(reader);
                break;

            case 91:
                item.femaleHeadModelIds[0] =
                    readOptionalId(reader);
                break;

            case 92:
                item.maleHeadModelIds[1] =
                    readOptionalId(reader);
                break;

            case 93:
                item.femaleHeadModelIds[1] =
                    readOptionalId(reader);
                break;

            case 95:
                item.rotationZ = reader.readU16();
                break;

            case 97:
                item.noteItemId =
                    readOptionalId(reader);
                break;

            case 98:
                item.noteTemplateId =
                    readOptionalId(reader);
                break;

            case 110:
                item.scaleX = reader.readU16();
                break;

            case 111:
                item.scaleY = reader.readU16();
                break;

            case 112:
                item.scaleZ = reader.readU16();
                break;

            case 113:
                item.ambient = reader.readI8();
                break;

            case 114:
                item.contrast = reader.readI8();
                break;

            case 115:
                item.team = reader.readU8();
                break;

            default:
                if (
                    opcode >= 30 &&
                    opcode < 35
                ) {
                    std::string action =
                        reader.readTerminatedString(10);

                    if (action != "hidden") {
                        item.groundActions[
                            opcode - 30
                        ] = std::move(action);
                    }

                    break;
                }

                if (
                    opcode >= 35 &&
                    opcode < 40
                ) {
                    item.inventoryActions[
                        opcode - 35
                    ] = reader.readTerminatedString(10);

                    break;
                }

                if (
                    opcode >= 100 &&
                    opcode < 110
                ) {
                    ItemStackVariant& variant =
                        item.stackVariants[
                            opcode - 100
                        ];

                    variant.itemId =
                        readOptionalId(reader);

                    variant.amount =
                        reader.readU16();

                    break;
                }

                throw std::runtime_error(
                    "Unknown item opcode " +
                    std::to_string(opcode)
                );
        }
    }

    throw std::runtime_error(
        "Item payload is missing terminator"
    );
}

}
