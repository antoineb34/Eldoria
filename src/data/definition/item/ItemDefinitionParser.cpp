#include "ItemDefinitionParser.h"

#include <exception>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::definition {

namespace {

std::optional<std::uint16_t> readOptionalId(
    eld::binary::ByteReader& reader
) {
    const std::uint16_t id =
        reader.readU16();

    return id == 65535
        ? std::nullopt
        : std::optional{id};
}

std::int16_t readSignedOffset(
    eld::binary::ByteReader& reader
) {
    return static_cast<std::int16_t>(
        reader.readU16()
    );
}

}

std::optional<ItemDefinition>
ItemDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        ItemDefinition definition;
        definition.id = record.id;

        while (!reader.atEnd()) {
            const std::uint8_t opcode =
                reader.readU8();

            switch (opcode) {
                case 0:
                    return reader.atEnd()
                        ? std::optional{definition}
                        : std::nullopt;

                case 1:
                    definition.inventoryModelId =
                        readOptionalId(reader);
                    break;

                case 2:
                    definition.name =
                        reader.readTerminatedString(10);
                    break;

                case 3:
                    definition.description =
                        reader.readTerminatedString(10);
                    break;

                case 4:
                    definition.zoom =
                        reader.readU16();
                    break;

                case 5:
                    definition.rotationX =
                        reader.readU16();
                    break;

                case 6:
                    definition.rotationY =
                        reader.readU16();
                    break;

                case 7:
                    definition.offsetX =
                        readSignedOffset(reader);
                    break;

                case 8:
                    definition.offsetY =
                        readSignedOffset(reader);
                    break;

                case 10:
                    reader.readU16();
                    break;

                case 11:
                    definition.stackable = true;
                    break;

                case 12:
                    definition.value =
                        reader.readU32();
                    break;

                case 16:
                    definition.membersOnly = true;
                    break;

                case 23:
                    definition.maleModelIds[0] =
                        readOptionalId(reader);

                    definition.maleModelOffset =
                        reader.readI8();
                    break;

                case 24:
                    definition.maleModelIds[1] =
                        readOptionalId(reader);
                    break;

                case 25:
                    definition.femaleModelIds[0] =
                        readOptionalId(reader);

                    definition.femaleModelOffset =
                        reader.readI8();
                    break;

                case 26:
                    definition.femaleModelIds[1] =
                        readOptionalId(reader);
                    break;

                case 40: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.recolors.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.recolors.push_back({
                            reader.readU16(),
                            reader.readU16()
                        });
                    }

                    break;
                }

                case 78:
                    definition.maleModelIds[2] =
                        readOptionalId(reader);
                    break;

                case 79:
                    definition.femaleModelIds[2] =
                        readOptionalId(reader);
                    break;

                case 90:
                    definition.maleHeadModelIds[0] =
                        readOptionalId(reader);
                    break;

                case 91:
                    definition.femaleHeadModelIds[0] =
                        readOptionalId(reader);
                    break;

                case 92:
                    definition.maleHeadModelIds[1] =
                        readOptionalId(reader);
                    break;

                case 93:
                    definition.femaleHeadModelIds[1] =
                        readOptionalId(reader);
                    break;

                case 95:
                    definition.rotationZ =
                        reader.readU16();
                    break;

                case 97:
                    definition.noteItemId =
                        readOptionalId(reader);
                    break;

                case 98:
                    definition.noteTemplateId =
                        readOptionalId(reader);
                    break;

                case 110:
                    definition.scaleX =
                        reader.readU16();
                    break;

                case 111:
                    definition.scaleY =
                        reader.readU16();
                    break;

                case 112:
                    definition.scaleZ =
                        reader.readU16();
                    break;

                case 113:
                    definition.ambient =
                        reader.readI8();
                    break;

                case 114:
                    definition.contrast =
                        reader.readI8();
                    break;

                case 115:
                    definition.team =
                        reader.readU8();
                    break;

                default:
                    if (
                        opcode >= 30 &&
                        opcode < 35
                    ) {
                        std::string action =
                            reader.readTerminatedString(10);

                        if (action != "hidden") {
                            definition.groundActions[
                                opcode - 30
                            ] = std::move(action);
                        }

                        break;
                    }

                    if (
                        opcode >= 35 &&
                        opcode < 40
                    ) {
                        definition.inventoryActions[
                            opcode - 35
                        ] = reader.readTerminatedString(10);
                        break;
                    }

                    if (
                        opcode >= 100 &&
                        opcode < 110
                    ) {
                        ItemStackVariant& variant =
                            definition.stackVariants[
                                opcode - 100
                            ];

                        variant.itemId =
                            readOptionalId(reader);

                        variant.amount =
                            reader.readU16();

                        break;
                    }

                    return std::nullopt;
            }
        }

        return std::nullopt;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
