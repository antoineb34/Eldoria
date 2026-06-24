#include "VarpDefinitionParser.h"

#include <exception>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<VarpDefinition>
VarpDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        VarpDefinition definition;
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
                    definition.opcode1Value =
                        reader.readU8();
                    break;

                case 2:
                    definition.opcode2Value =
                        reader.readU8();
                    break;

                case 3:
                    definition.tracked = true;
                    break;

                case 4:
                    definition.persistent = false;
                    break;

                case 5:
                    definition.clientCode =
                        reader.readU16();
                    break;

                case 6:
                    definition.opcode6Flag = true;
                    break;

                case 7:
                    definition.opcode7Value =
                        reader.readU32();
                    break;

                case 8:
                    definition.active = true;
                    definition.mode = 1;
                    break;

                case 10:
                    definition.name =
                        reader.readTerminatedString(10);
                    break;

                case 11:
                    definition.active = true;
                    break;

                case 12:
                    definition.opcode12Value =
                        reader.readU32();
                    break;

                case 13:
                    definition.mode = 2;
                    break;

                default:
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
