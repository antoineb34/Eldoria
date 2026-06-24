#include "VarbitDefinitionParser.h"

#include <exception>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<VarbitDefinition>
VarbitDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        VarbitDefinition definition;
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
                    definition.varpId =
                        reader.readU16();

                    definition.leastSignificantBit =
                        reader.readU8();

                    definition.mostSignificantBit =
                        reader.readU8();
                    break;

                case 2:
                    definition.tracked = true;
                    break;

                case 3:
                    definition.opcode3Value =
                        reader.readU32();
                    break;

                case 4:
                    definition.opcode4Value =
                        reader.readU32();
                    break;

                case 10:
                    definition.name =
                        reader.readTerminatedString(10);
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
